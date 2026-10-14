package com.mathcastilho.naviobridge;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothStatusCodes;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.ParcelUuid;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

public final class BleNavigationService extends Service {
    static final String PREFERENCES = "navio_bridge";
    static final String PREF_ENABLED = "relay_enabled";
    static final String ACTION_START = "com.mathcastilho.naviobridge.START";
    static final String ACTION_STOP = "com.mathcastilho.naviobridge.STOP";
    static final String ACTION_NAVIGATION = "com.mathcastilho.naviobridge.NAVIGATION";
    static final String ACTION_NAVIGATION_STOP = "com.mathcastilho.naviobridge.NAVIGATION_STOP";
    private static final String CHANNEL_ID = "navio_bridge_status";
    private static final int NOTIFICATION_ID = 1001;
    private static final long SCAN_WINDOW_MS = 12000;
    private static final long RETRY_DELAY_MS = 3000;
    private static final UUID SERVICE_UUID =
            UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    private static final UUID RX_UUID =
            UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
    private static final UUID TX_UUID =
            UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    private static final UUID CCCD_UUID =
            UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private final Handler handler = new Handler(Looper.getMainLooper());
    private final ArrayDeque<byte[]> writes = new ArrayDeque<>();
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothLeScanner scanner;
    private BluetoothGatt gatt;
    private BluetoothGattCharacteristic rxCharacteristic;
    private BluetoothGattCharacteristic txCharacteristic;
    private boolean scanRunning;
    private boolean writeInProgress;
    private boolean foregroundStarted;
    private int negotiatedMtu = 23;
    private NavigationText currentNavigation;
    private byte[] currentSettingsPacket;

    private final Runnable scanTimeout = () -> {
        stopScan();
        updateForegroundNotification("ESP32 not found; retrying");
        scheduleScan(RETRY_DELAY_MS);
    };

    private final Runnable reconnect = () -> {
        if (relayEnabled() && gatt == null) {
            scanForEsp();
        }
    };

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            if (result.getScanRecord() == null
                    || result.getScanRecord().getServiceUuids() == null
                    || result.getScanRecord().getServiceUuids().stream()
                    .noneMatch(uuid -> SERVICE_UUID.equals(uuid.getUuid()))) {
                return;
            }
            stopScan();
            updateForegroundNotification("Connecting to " + safeDeviceName(device));
            connect(device);
        }

        @Override
        public void onScanFailed(int errorCode) {
            scanRunning = false;
            updateForegroundNotification("Bluetooth scan failed (" + errorCode + ")");
            scheduleScan(RETRY_DELAY_MS);
        }
    };

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt bluetoothGatt, int status, int newState) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                closeGatt(bluetoothGatt);
                updateForegroundNotification("ESP32 disconnected; reconnecting");
                scheduleScan(RETRY_DELAY_MS);
                return;
            }
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                gatt = bluetoothGatt;
                updateForegroundNotification("Connected; setting up navigation relay");
                if (!bluetoothGatt.discoverServices()) {
                    disconnectAndRetry();
                }
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                closeGatt(bluetoothGatt);
                updateForegroundNotification("ESP32 disconnected; reconnecting");
                scheduleScan(RETRY_DELAY_MS);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt bluetoothGatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                disconnectAndRetry();
                return;
            }
            BluetoothGattService service = bluetoothGatt.getService(SERVICE_UUID);
            if (service == null) {
                disconnectAndRetry();
                return;
            }
            rxCharacteristic = service.getCharacteristic(RX_UUID);
            txCharacteristic = service.getCharacteristic(TX_UUID);
            if (rxCharacteristic == null || txCharacteristic == null) {
                disconnectAndRetry();
                return;
            }

            try {
                bluetoothGatt.setCharacteristicNotification(txCharacteristic, true);
                BluetoothGattDescriptor descriptor = txCharacteristic.getDescriptor(CCCD_UUID);
                if (descriptor != null) {
                    boolean queued;
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                        queued = bluetoothGatt.writeDescriptor(
                                descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)
                                == BluetoothStatusCodes.SUCCESS;
                    } else {
                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        queued = bluetoothGatt.writeDescriptor(descriptor);
                    }
                    if (queued) {
                        return;
                    }
                }
                requestMtu(bluetoothGatt);
            } catch (SecurityException exception) {
                showError("Bluetooth permission was revoked");
            }
        }

        @Override
        public void onDescriptorWrite(
                BluetoothGatt bluetoothGatt, BluetoothGattDescriptor descriptor, int status) {
            requestMtu(bluetoothGatt);
        }

        @Override
        public void onMtuChanged(BluetoothGatt bluetoothGatt, int mtu, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                negotiatedMtu = mtu;
            }
            updateForegroundNotification("Connected to Chronos Navio");
            sendCurrentDeviceState();
        }

        private void sendCurrentDeviceState() {
            if (currentSettingsPacket == null) {
                currentSettingsPacket = DeviceSettings.load(BleNavigationService.this).toPacket();
            }
            enqueuePacket(currentSettingsPacket);
            if (currentNavigation != null) {
                enqueuePackets(ChronosNavigationPacket.activePackets(currentNavigation));
            }
        }

        @Override
        public void onCharacteristicWrite(
                BluetoothGatt bluetoothGatt, BluetoothGattCharacteristic characteristic, int status) {
            writeInProgress = false;
            if (status != BluetoothGatt.GATT_SUCCESS) {
                updateForegroundNotification("Navigation update failed");
            }
            writeNext();
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        BluetoothManager manager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
        bluetoothAdapter = manager == null ? null : manager.getAdapter();
        currentSettingsPacket = DeviceSettings.load(this).toPacket();
        createNotificationChannel();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent == null ? ACTION_START : intent.getAction();
        if (ACTION_STOP.equals(action)) {
            stopRelay();
            return START_NOT_STICKY;
        }

        if (ACTION_START.equals(action) || intent == null) {
            currentSettingsPacket = DeviceSettings.load(this).toPacket();
            startForegroundNotification("Starting Bluetooth connection");
            scanForEsp();
            return START_STICKY;
        }

        startForegroundNotification("Navigation relay active");
        if (DeviceSettings.ACTION_UPDATE.equals(action)) {
            byte[] packet = intent.getByteArrayExtra(DeviceSettings.EXTRA_PACKET);
            if (packet != null && relayEnabled()) {
                currentSettingsPacket = packet.clone();
                if (gatt != null && rxCharacteristic != null) {
                    enqueuePacket(currentSettingsPacket);
                }
            }
        } else if (ACTION_NAVIGATION.equals(action)) {
            NavigationText navigationText = NavigationText.fromIntent(intent);
            if (navigationText.hasContent() && relayEnabled()) {
                currentNavigation = navigationText;
                if (gatt != null && rxCharacteristic != null) {
                    enqueuePackets(ChronosNavigationPacket.activePackets(currentNavigation));
                } else {
                    scanForEsp();
                }
            }
        } else if (ACTION_NAVIGATION_STOP.equals(action) && relayEnabled()) {
            currentNavigation = null;
            if (gatt != null && rxCharacteristic != null) {
                enqueuePacket(ChronosNavigationPacket.inactive());
            }
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        stopScan();
        handler.removeCallbacksAndMessages(null);
        if (gatt != null) {
            closeGatt(gatt);
        }
        super.onDestroy();
    }

    private void scanForEsp() {
        if (!relayEnabled() || scanRunning || gatt != null) {
            return;
        }
        if (!hasBluetoothPermissions()) {
            showError("Bluetooth permission is required");
            return;
        }

        try {
            if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
                updateForegroundNotification("Turn on Bluetooth to connect");
                scheduleScan(RETRY_DELAY_MS);
                return;
            }
            scanner = bluetoothAdapter.getBluetoothLeScanner();
            if (scanner == null) {
                updateForegroundNotification("Bluetooth scanner unavailable");
                scheduleScan(RETRY_DELAY_MS);
                return;
            }
            ScanFilter filter = new ScanFilter.Builder()
                    .setServiceUuid(new ParcelUuid(SERVICE_UUID))
                    .build();
            ScanSettings settings = new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .build();
            scanner.startScan(Arrays.asList(filter), settings, scanCallback);
            scanRunning = true;
            updateForegroundNotification("Searching for Chronos Navio");
            handler.removeCallbacks(scanTimeout);
            handler.postDelayed(scanTimeout, SCAN_WINDOW_MS);
        } catch (SecurityException exception) {
            showError("Bluetooth scan permission was revoked");
        }
    }

    private void connect(BluetoothDevice device) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                gatt = device.connectGatt(this, false, gattCallback, BluetoothDevice.TRANSPORT_LE);
            } else {
                gatt = device.connectGatt(this, false, gattCallback);
            }
            if (gatt == null) {
                scheduleScan(RETRY_DELAY_MS);
            }
        } catch (SecurityException exception) {
            showError("Bluetooth connect permission was revoked");
        }
    }

    private void requestMtu(BluetoothGatt bluetoothGatt) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP
                    && bluetoothGatt.requestMtu(517)) {
                return;
            }
            negotiatedMtu = 23;
            updateForegroundNotification("Connected to Chronos Navio");
            if (currentSettingsPacket == null) {
                currentSettingsPacket = DeviceSettings.load(this).toPacket();
            }
            enqueuePacket(currentSettingsPacket);
            if (currentNavigation != null) {
                enqueuePackets(ChronosNavigationPacket.activePackets(currentNavigation));
            }
        } catch (SecurityException exception) {
            showError("Bluetooth permission was revoked");
        }
    }

    private void enqueuePacket(byte[] packet) {
        enqueuePackets(Arrays.asList(packet));
    }

    private void enqueuePackets(List<byte[]> packets) {
        for (byte[] packet : packets) {
            writes.addAll(fragment(packet));
        }
        writeNext();
    }

    private List<byte[]> fragment(byte[] packet) {
        ArrayList<byte[]> fragments = new ArrayList<>();
        if (packet.length <= negotiatedMtu - 3) {
            fragments.add(packet);
            return fragments;
        }

        int offset = Math.min(20, packet.length);
        fragments.add(Arrays.copyOfRange(packet, 0, offset));
        int sequence = 0;
        while (offset < packet.length) {
            int dataLength = Math.min(19, packet.length - offset);
            byte[] fragment = new byte[dataLength + 1];
            fragment[0] = (byte) sequence++;
            System.arraycopy(packet, offset, fragment, 1, dataLength);
            fragments.add(fragment);
            offset += dataLength;
        }
        return fragments;
    }

    private void writeNext() {
        if (writeInProgress || writes.isEmpty() || gatt == null || rxCharacteristic == null) {
            return;
        }
        byte[] next = writes.removeFirst();
        try {
            boolean started;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                started = gatt.writeCharacteristic(
                        rxCharacteristic,
                        next,
                        BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
                        == BluetoothStatusCodes.SUCCESS;
            } else {
                rxCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                rxCharacteristic.setValue(next);
                started = gatt.writeCharacteristic(rxCharacteristic);
            }
            writeInProgress = started;
            if (!started) {
                updateForegroundNotification("Could not send navigation update");
                handler.post(this::writeNext);
            }
        } catch (SecurityException exception) {
            showError("Bluetooth write permission was revoked");
        }
    }

    private void disconnectAndRetry() {
        if (gatt != null) {
            closeGatt(gatt);
        }
        updateForegroundNotification("ESP32 setup failed; reconnecting");
        scheduleScan(RETRY_DELAY_MS);
    }

    private void scheduleScan(long delayMillis) {
        handler.removeCallbacks(reconnect);
        handler.postDelayed(reconnect, delayMillis);
    }

    private void stopScan() {
        handler.removeCallbacks(scanTimeout);
        if (scanner != null && scanRunning) {
            try {
                scanner.stopScan(scanCallback);
            } catch (SecurityException exception) {
                updateForegroundNotification("Bluetooth scan permission was revoked");
            }
        }
        scanRunning = false;
    }

    private void closeGatt(BluetoothGatt bluetoothGatt) {
        if (gatt == bluetoothGatt) {
            gatt = null;
            rxCharacteristic = null;
            txCharacteristic = null;
            writes.clear();
            writeInProgress = false;
        }
        try {
            bluetoothGatt.close();
        } catch (SecurityException exception) {
            updateForegroundNotification("Bluetooth connect permission was revoked");
        }
    }

    private boolean hasBluetoothPermissions() {
        return Build.VERSION.SDK_INT < Build.VERSION_CODES.S
                || checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN)
                == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT)
                == PackageManager.PERMISSION_GRANTED;
    }

    private boolean relayEnabled() {
        return getSharedPreferences(PREFERENCES, MODE_PRIVATE)
                .getBoolean(PREF_ENABLED, false);
    }

    private String safeDeviceName(BluetoothDevice device) {
        try {
            String name = device.getName();
            return name == null || name.isEmpty() ? "Chronos Navio" : name;
        } catch (SecurityException exception) {
            return "Chronos Navio";
        }
    }

    private void stopRelay() {
        getSharedPreferences(PREFERENCES, MODE_PRIVATE)
                .edit().putBoolean(PREF_ENABLED, false).apply();
        stopScan();
        handler.removeCallbacks(reconnect);
        currentNavigation = null;
        if (gatt != null) {
            closeGatt(gatt);
        }
        stopForeground(STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    private void showError(String message) {
        updateForegroundNotification(message);
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID,
                    "Navio Bridge status",
                    NotificationManager.IMPORTANCE_MIN);
            channel.setDescription(
                    "Quiet status for the Bluetooth connection and navigation relay");
            channel.setShowBadge(false);
            channel.setSound(null, null);
            channel.enableVibration(false);
            channel.enableLights(false);
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }

    private void startForegroundNotification(String message) {
        Notification notification = buildNotification(message);
        if (foregroundStarted) {
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.notify(NOTIFICATION_ID, notification);
            }
            return;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            startForeground(
                    NOTIFICATION_ID,
                    notification,
                    android.content.pm.ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
        } else {
            startForeground(NOTIFICATION_ID, notification);
        }
        foregroundStarted = true;
    }

    private void updateForegroundNotification(String message) {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.notify(NOTIFICATION_ID, buildNotification(message));
        }
    }

    private Notification buildNotification(String message) {
        Intent stopIntent = new Intent(this, BleNavigationService.class)
                .setAction(ACTION_STOP);
        PendingIntent stopPendingIntent = PendingIntent.getService(
                this,
                1,
                stopIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID);
        } else {
            builder = new Notification.Builder(this);
        }
        return builder
                .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
                .setContentTitle("Navio Bridge")
                .setContentText(message)
                .setPriority(Notification.PRIORITY_MIN)
                .setDefaults(0)
                .setOngoing(true)
                .addAction(new Notification.Action.Builder(
                        null, "Stop", stopPendingIntent).build())
                .build();
    }
}
