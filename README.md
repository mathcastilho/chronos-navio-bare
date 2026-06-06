# Chronos Navio

Chronos Navio is an ESP32 navigation display firmware built with Arduino,
PlatformIO, LVGL, and [ChronosESP32](https://github.com/fbiego/ChronosESP32).
It receives navigation and device information over Bluetooth and presents it on
a range of LCD and AMOLED boards.

The project uses compile-time board profiles so the application can support
different displays, touch controllers, buttons, backlights, and board-specific
initialization without adding hardware checks throughout the main application.

## Features

- Bluetooth navigation data through ChronosESP32
- LVGL navigation and settings interface
- LCD and AMOLED display support
- Arduino_GFX, LovyanGFX, and M5Stack display adapters
- Optional touch, buttons, encoders, and custom board hooks
- Configurable rotation, touch rotation, and backlight polarity
- Screen timeout and wake-input support
- Persistent settings through ESP32 Preferences
- Per-board merged firmware binaries


## Supported Environments

| PlatformIO environment | Board |
| --- | --- |
| `elecrow-c3-128` | Elecrow C3 1.28-inch LCD |
| `viewe-smartring` | Viewe SmartRing 466x466 AMOLED |
| `viewe-knob-15` | Viewe Touch Knob 1.5-inch AMOLED |
| `viewe-s3-15` | Viewe S3 1.5-inch AMOLED |
| `viewe-echo-ear` | Viewe Echo Ear 360x360 LCD |
| `viewe-knob-128` | Viewe Knob 1.28-inch LCD |
| `viewe-s3-28` | Viewe S3 2.8-inch LCD |
| `m5-dial` | M5Stack Dial |
| `m5-core-basic` | M5Stack Core Basic |
| `m5-core-s3-lite` | M5Stack Core S3 Lite |
| `m5-core-s3-se` | M5Stack Core S3 SE |
| `m5-cardputer` | M5Stack Cardputer |
| `m5-cardputer-adv` | M5Stack Cardputer Adv |
| `m5-stick-c` | M5Stack Stick C |
| `m5-atom-s3r` | M5Stack Atom S3R |
| `guition-2424s012` | Guition 2424S012 |
| `guition-2432s028r` | Guition 2432S028R / CYD |
| `waveshare-s3-lcd-128` | Waveshare ESP32-S3 Touch LCD 1.28 |
| `waveshare-s3-lcd-169` | Waveshare ESP32-S3 Touch LCD 1.69 |
| `waveshare-s3-amoled-206` | Waveshare ESP32-S3 Touch AMOLED 2.06 |
| `hwlab-webscreen` | HW Media Lab Webscreen |

## Firmware

Prebuilt firmware is available for the listed boards on ESPVerse

<a href="https://espverse.com/missions/chronos-navio"><img src="https://espverse.com/assets/images/espverse_badge.png" alt="Flashable on ESPVerse" width="200"></a>

## License

The project source is provided under the MIT License. See the license notice in
the source files.
