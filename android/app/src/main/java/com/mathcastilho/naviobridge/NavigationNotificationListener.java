package com.mathcastilho.naviobridge;

import android.app.Notification;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public final class NavigationNotificationListener extends NotificationListenerService {
    static final String ACTION_RELAY_CURRENT =
            "com.mathcastilho.naviobridge.RELAY_CURRENT";
    private static final Set<String> NAVIGATION_PACKAGES = new HashSet<>(Arrays.asList(
            "com.google.android.apps.maps",
            "com.waze",
            "net.osmand",
            "net.osmand.plus"));
    private static final long REMOVAL_GRACE_MS = 2000;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private final Runnable endNavigation = this::endIfNoNavigationNotifications;
    private boolean receiverRegistered;
    private final BroadcastReceiver requestCurrentNavigation = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            relayCurrentNotifications();
        }
    };

    @Override
    public void onListenerConnected() {
        super.onListenerConnected();
        if (!receiverRegistered) {
            IntentFilter filter = new IntentFilter(ACTION_RELAY_CURRENT);
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
                registerReceiver(requestCurrentNavigation, filter, Context.RECEIVER_NOT_EXPORTED);
            } else {
                registerReceiver(requestCurrentNavigation, filter);
            }
            receiverRegistered = true;
        }
        relayCurrentNotifications();
    }

    @Override
    public void onListenerDisconnected() {
        if (receiverRegistered) {
            unregisterReceiver(requestCurrentNavigation);
            receiverRegistered = false;
        }
        super.onListenerDisconnected();
        requestRebind(new ComponentName(this, NavigationNotificationListener.class));
    }

    private void relayCurrentNotifications() {
        StatusBarNotification[] active = getActiveNotifications();
        if (active == null) {
            return;
        }
        for (StatusBarNotification notification : active) {
            relayIfNavigation(notification);
        }
    }

    @Override
    public void onNotificationPosted(StatusBarNotification notification) {
        relayIfNavigation(notification);
    }

    @Override
    public void onNotificationRemoved(StatusBarNotification notification) {
        if (NAVIGATION_PACKAGES.contains(notification.getPackageName())) {
            handler.removeCallbacks(endNavigation);
            handler.postDelayed(endNavigation, REMOVAL_GRACE_MS);
        }
    }

    private boolean relayIfNavigation(StatusBarNotification statusBarNotification) {
        String packageName = statusBarNotification.getPackageName();
        Notification notification = statusBarNotification.getNotification();
        if (!NAVIGATION_PACKAGES.contains(packageName)
                || !isOngoingNavigation(notification)) {
            return false;
        }

        NavigationText navigationText = extractNavigationText(notification);
        if (!navigationText.hasContent()) {
            return false;
        }

        handler.removeCallbacks(endNavigation);
        Intent intent = new Intent(this, BleNavigationService.class)
                .setAction(BleNavigationService.ACTION_NAVIGATION)
                .putExtra(NavigationText.EXTRA_TITLE, navigationText.title)
                .putExtra(NavigationText.EXTRA_DURATION, navigationText.duration)
                .putExtra(NavigationText.EXTRA_DISTANCE, navigationText.distance)
                .putExtra(NavigationText.EXTRA_ETA, navigationText.eta)
                .putExtra(NavigationText.EXTRA_DIRECTIONS, navigationText.directions)
                .putExtra(NavigationText.EXTRA_SPEED, navigationText.speed)
                .putExtra(NavigationText.EXTRA_ICON,
                        NavigationIcon.fromNotification(this, notification));
        startRelayService(intent);
        return true;
    }

    private NavigationText extractNavigationText(Notification notification) {
        List<String> values = new ArrayList<>();
        addValue(values, notification.extras.getCharSequence(Notification.EXTRA_TITLE));
        addValue(values, notification.extras.getCharSequence(Notification.EXTRA_TITLE_BIG));
        addValue(values, notification.extras.getCharSequence(Notification.EXTRA_TEXT));
        addValue(values, notification.extras.getCharSequence(Notification.EXTRA_BIG_TEXT));
        addValue(values, notification.extras.getCharSequence(Notification.EXTRA_SUB_TEXT));

        CharSequence[] lines =
                notification.extras.getCharSequenceArray(Notification.EXTRA_TEXT_LINES);
        if (lines != null) {
            for (CharSequence line : lines) {
                addValue(values, line);
            }
        }
        return NavigationTextParser.parse(values);
    }

    private void addValue(List<String> values, CharSequence value) {
        if (value == null) {
            return;
        }
        String text = value.toString().trim();
        if (!text.isEmpty() && !values.contains(text)) {
            values.add(text);
        }
    }

    private void endIfNoNavigationNotifications() {
        if (!isEnabledForRelay()) {
            return;
        }

        StatusBarNotification[] active = getActiveNotifications();
        if (active != null) {
            for (StatusBarNotification notification : active) {
                if (NAVIGATION_PACKAGES.contains(notification.getPackageName())
                        && isOngoingNavigation(notification.getNotification())) {
                    if (relayIfNavigation(notification)) {
                        return;
                    }
                }
            }
        }

        startRelayService(new Intent(this, BleNavigationService.class)
                .setAction(BleNavigationService.ACTION_NAVIGATION_STOP));
    }

    private boolean isOngoingNavigation(Notification notification) {
        return (notification.flags & Notification.FLAG_ONGOING_EVENT) != 0
                || Notification.CATEGORY_NAVIGATION.equals(notification.category);
    }

    private boolean isEnabledForRelay() {
        return getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE)
                .getBoolean(BleNavigationService.PREF_ENABLED, false);
    }

    private void startRelayService(Intent intent) {
        if (!isEnabledForRelay()) {
            return;
        }
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            startForegroundService(intent);
        } else {
            startService(intent);
        }
    }
}
