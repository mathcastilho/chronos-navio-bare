# Navio Bridge for Android

Navio Bridge relays ongoing direction notifications from Google Maps, Waze, or
OsmAnd to Chronos Navio over Bluetooth LE. It connects to the ESP32's existing
ChronosESP32 BLE service, so this companion app does not require a firmware
change.

## Open and build

Open the `android` directory in Android Studio and let Gradle sync. The project
uses Android Gradle Plugin 8.7.3, Java 17, and Android SDK 35.

## First use

1. Install and open Navio Bridge.
2. Grant notification access to **Navio Bridge navigation relay** in Android
   Settings. This permission is required for the app to read navigation updates.
3. Grant the requested Bluetooth (and notification) permissions.
4. Turn on the ESP32 and tap **Connect and start relaying**.
5. Start turn-by-turn navigation in Google Maps, Waze, or OsmAnd.

Android requires a foreground-service notification while the Bluetooth bridge
is running. Navio Bridge puts it on a quiet, minimum-importance channel with no
sound, vibration, lights, or launcher badge; depending on the phone, it may be
shown only in the notification shade or under silent notifications. Android
does not let an app fully hide this notification while the foreground service
is active. Use its **Stop** action or the app's **Stop relay** button to
disconnect.

On Android 11 and earlier, location permission is required by Android to scan
for Bluetooth LE devices. Navio Bridge does not request or use location data.

## ESP32 display settings

Open **Chronos Navio settings** in the app to change brightness, screen timeout,
icon size, ETA visibility,
direction visibility and size, trip-information visibility, maneuver heading
position, and theme color. Changes are saved on the phone and sent immediately
when the BLE relay is connected; otherwise they are sent the next time Navio
Bridge connects. The ESP32 stores received settings in Preferences, so they
remain active after a reboot.

This requires the matching firmware, which receives the app-specific settings
frame over the existing ChronosESP32 BLE RX characteristic. Older firmware
continues to use its built-in defaults and ignores that frame.

## Supported sources and protocol

The listener forwards ongoing notifications from Google Maps, Waze, OsmAnd,
and OsmAnd+. The app separates maneuver text from any trip time, destination
distance, and ETA that the notification exposes, then sends those fields in
the ChronosESP32 protocol's expected order. It does not calculate routes or
access map/location APIs. Notification layouts vary between navigation apps
and versions, so unavailable trip values are sent empty rather than guessed.
When the notification exposes a large icon, the bridge converts that graphic
to the ESP32's 48x48 monochrome format and sends it in the protocol's three
icon-data packets. If no large icon is available, it falls back to a simple
left, right, or straight glyph inferred from the maneuver text.

The ESP32 firmware may hide ETA even when the bridge sends it: the current
firmware configuration disables arrival-time display.

The ESP32 is discovered by the ChronosESP32 service UUID and receives
navigation frames on its existing RX characteristic. BLE packet framing and
the navigation payload fields follow the ChronosESP32 library used by this
firmware.
