package com.mathcastilho.naviobridge;

import android.content.Context;
import android.content.SharedPreferences;

final class DeviceSettings {
    static final String ACTION_UPDATE = "com.mathcastilho.naviobridge.DEVICE_SETTINGS";
    static final String EXTRA_PACKET = "device_settings_packet";
    static final int DEFAULT_BRIGHTNESS = 100;
    static final int DEFAULT_TIMEOUT = 3;
    static final int DEFAULT_ICON_SIZE = 384;
    static final boolean DEFAULT_SHOW_ETA = false;
    static final boolean DEFAULT_SHOW_DIRECTIONS = true;
    static final boolean DEFAULT_LARGE_DIRECTIONS = true;
    static final int DEFAULT_THEME_COLOR = 0xFFFFFF;
    static final boolean DEFAULT_SHOW_TRIP_INFO = true;
    static final boolean DEFAULT_HEADING_ABOVE_ICON = false;

    int brightness = DEFAULT_BRIGHTNESS;
    int timeout = DEFAULT_TIMEOUT;
    int iconSize = DEFAULT_ICON_SIZE;
    boolean showEta = DEFAULT_SHOW_ETA;
    boolean showDirections = DEFAULT_SHOW_DIRECTIONS;
    boolean largeDirections = DEFAULT_LARGE_DIRECTIONS;
    int themeColor = DEFAULT_THEME_COLOR;
    boolean showTripInfo = DEFAULT_SHOW_TRIP_INFO;
    boolean headingAboveIcon = DEFAULT_HEADING_ABOVE_ICON;

    static DeviceSettings load(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(
                BleNavigationService.PREFERENCES, Context.MODE_PRIVATE);
        DeviceSettings settings = new DeviceSettings();
        settings.brightness = preferences.getInt("device_brightness", DEFAULT_BRIGHTNESS);
        settings.timeout = preferences.getInt("device_timeout", DEFAULT_TIMEOUT);
        settings.iconSize = preferences.getInt("device_icon_size", DEFAULT_ICON_SIZE);
        settings.showEta = preferences.getBoolean("device_show_eta", DEFAULT_SHOW_ETA);
        settings.showDirections = preferences.getBoolean(
                "device_show_directions", DEFAULT_SHOW_DIRECTIONS);
        settings.largeDirections = preferences.getBoolean(
                "device_large_directions", DEFAULT_LARGE_DIRECTIONS);
        settings.themeColor = preferences.getInt("device_theme_color", DEFAULT_THEME_COLOR);
        settings.showTripInfo = preferences.getBoolean(
                "device_show_trip_info", DEFAULT_SHOW_TRIP_INFO);
        settings.headingAboveIcon = preferences.getBoolean(
                "device_heading_above_icon", DEFAULT_HEADING_ABOVE_ICON);
        return settings;
    }

    void save(Context context) {
        context.getSharedPreferences(BleNavigationService.PREFERENCES, Context.MODE_PRIVATE)
                .edit()
                .putInt("device_brightness", brightness)
                .putInt("device_timeout", timeout)
                .putInt("device_icon_size", iconSize)
                .putBoolean("device_show_eta", showEta)
                .putBoolean("device_show_directions", showDirections)
                .putBoolean("device_large_directions", largeDirections)
                .putInt("device_theme_color", themeColor)
                .putBoolean("device_show_trip_info", showTripInfo)
                .putBoolean("device_heading_above_icon", headingAboveIcon)
                .remove("device_language")
                .remove("device_rotation")
                .remove("device_hr24")
                .remove("device_show_time")
                .apply();
    }

    byte[] toPacket() {
        // Versioned app-specific Chronos frame; the firmware validates all values.
        byte[] packet = new byte[19];
        packet[0] = (byte) 0xAB;
        packet[1] = 0;
        packet[2] = 16;
        packet[3] = (byte) 0xFE;
        packet[4] = 0x7E;
        packet[5] = 3;
        packet[6] = (byte) brightness;
        packet[7] = (byte) timeout;
        packet[8] = (byte) ((iconSize >>> 8) & 0xFF);
        packet[9] = (byte) (iconSize & 0xFF);
        packet[10] = (byte) (showEta ? 1 : 0);
        packet[11] = (byte) (showDirections ? 1 : 0);
        packet[12] = (byte) (largeDirections ? 1 : 0);
        packet[13] = (byte) ((themeColor >>> 24) & 0xFF);
        packet[14] = (byte) ((themeColor >>> 16) & 0xFF);
        packet[15] = (byte) ((themeColor >>> 8) & 0xFF);
        packet[16] = (byte) (themeColor & 0xFF);
        packet[17] = (byte) (showTripInfo ? 1 : 0);
        packet[18] = (byte) (headingAboveIcon ? 1 : 0);
        return packet;
    }
}
