package com.mathcastilho.naviobridge;

import android.Manifest;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public final class MainActivity extends Activity {
    private static final int REQUEST_PERMISSIONS = 1;
    private TextView permissionStatus;
    private Button startButton;
    private Button stopButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        buildScreen();
    }

    @Override
    protected void onResume() {
        super.onResume();
        refreshStatus();
    }

    private void buildScreen() {
        int padding = dp(24);
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(padding, padding, padding, padding);

        TextView title = new TextView(this);
        title.setText("Navio Bridge");
        title.setTextSize(28);
        title.setTextColor(0xFF17202A);
        content.addView(title);

        TextView description = new TextView(this);
        description.setText(
                "Relay turn-by-turn notifications from Google Maps, Waze, or OsmAnd "
                        + "to your Chronos Navio display over Bluetooth LE.");
        description.setTextSize(16);
        description.setTextColor(0xFF46515C);
        description.setPadding(0, dp(12), 0, dp(20));
        content.addView(description);

        Button notificationAccess = new Button(this);
        notificationAccess.setText("Grant notification access");
        notificationAccess.setOnClickListener(view ->
                startActivity(new Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS)));
        content.addView(notificationAccess);

        permissionStatus = new TextView(this);
        permissionStatus.setTextSize(14);
        permissionStatus.setTextColor(0xFF46515C);
        permissionStatus.setPadding(0, dp(12), 0, dp(16));
        content.addView(permissionStatus);

        startButton = new Button(this);
        startButton.setText("Connect and start relaying");
        startButton.setOnClickListener(view -> startRelay());
        content.addView(startButton);

        stopButton = new Button(this);
        stopButton.setText("Stop relay");
        stopButton.setOnClickListener(view -> stopRelay());
        content.addView(stopButton);

        Button deviceSettingsButton = new Button(this);
        deviceSettingsButton.setText("Chronos Navio settings");
        deviceSettingsButton.setOnClickListener(view ->
                startActivity(new Intent(this, DeviceSettingsActivity.class)));
        content.addView(deviceSettingsButton);

        TextView note = new TextView(this);
        note.setText(
                "Notification access lets the relay read ongoing notifications from supported "
                        + "navigation apps. It sends their direction text to the paired ESP32; "
                        + "it does not read notifications from other apps.");
        note.setTextSize(13);
        note.setTextColor(0xFF697681);
        note.setPadding(0, dp(20), 0, 0);
        content.addView(note);

        ScrollView scrollView = new ScrollView(this);
        scrollView.setFillViewport(true);
        scrollView.addView(content);
        setContentView(scrollView);
        refreshStatus();
    }

    private void startRelay() {
        if (!hasNotificationAccess()) {
            startActivity(new Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS));
            return;
        }

        List<String> missingPermissions = missingRuntimePermissions();
        if (!missingPermissions.isEmpty()) {
            requestPermissions(
                    missingPermissions.toArray(new String[0]), REQUEST_PERMISSIONS);
            return;
        }

        SharedPreferences preferences =
                getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE);
        preferences.edit().putBoolean(BleNavigationService.PREF_ENABLED, true).apply();
        Intent serviceIntent = new Intent(this, BleNavigationService.class)
                .setAction(BleNavigationService.ACTION_START);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(serviceIntent);
        } else {
            startService(serviceIntent);
        }
        sendBroadcast(new Intent(NavigationNotificationListener.ACTION_RELAY_CURRENT)
                .setPackage(getPackageName()));
        refreshStatus();
    }

    private void stopRelay() {
        getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE)
                .edit().putBoolean(BleNavigationService.PREF_ENABLED, false).apply();
        startService(new Intent(this, BleNavigationService.class)
                .setAction(BleNavigationService.ACTION_STOP));
        refreshStatus();
    }

    private void refreshStatus() {
        if (permissionStatus == null) {
            return;
        }

        boolean notificationAccess = hasNotificationAccess();
        boolean bluetoothPermissions = missingRuntimePermissions().isEmpty();
        permissionStatus.setText(
                "Notification access: " + (notificationAccess ? "granted" : "needed")
                        + "\nBluetooth permissions: "
                        + (bluetoothPermissions ? "granted" : "needed")
                        + "\nESP32: " + (relayEnabled() ? "connecting / active" : "not connected"));
        startButton.setEnabled(notificationAccess && bluetoothPermissions);
        stopButton.setEnabled(relayEnabled());
    }

    private boolean relayEnabled() {
        return getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE)
                .getBoolean(BleNavigationService.PREF_ENABLED, false);
    }

    private boolean hasNotificationAccess() {
        String enabledListeners =
                Settings.Secure.getString(getContentResolver(), "enabled_notification_listeners");
        if (TextUtils.isEmpty(enabledListeners)) {
            return false;
        }
        ComponentName listener = new ComponentName(this, NavigationNotificationListener.class);
        for (String entry : enabledListeners.split(":")) {
            ComponentName enabled = ComponentName.unflattenFromString(entry);
            if (listener.equals(enabled)) {
                return true;
            }
        }
        return false;
    }

    private List<String> missingRuntimePermissions() {
        ArrayList<String> missing = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            addIfMissing(missing, Manifest.permission.BLUETOOTH_SCAN);
            addIfMissing(missing, Manifest.permission.BLUETOOTH_CONNECT);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            addIfMissing(missing, Manifest.permission.ACCESS_FINE_LOCATION);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            addIfMissing(missing, Manifest.permission.POST_NOTIFICATIONS);
        }
        return missing;
    }

    private void addIfMissing(List<String> permissions, String permission) {
        if (checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission);
        }
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS) {
            refreshStatus();
        }
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }
}
