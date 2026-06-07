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

## Firmware

Prebuilt firmware is available for the listed boards on ESPVerse

<a href="https://espverse.com/missions/chronos-navio"><img src="https://espverse.com/assets/images/espverse_badge.png" alt="Flashable on ESPVerse" width="200"></a>

## License

The project source is provided under the MIT License. See the license notice in
the source files.
