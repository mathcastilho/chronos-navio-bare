/*
   MIT License

  Copyright (c) 2025 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#include <Arduino.h>
#include <ChronosESP32.h>
#include <Preferences.h>
#include <cstdint>
#include <stdlib.h>
#include <timber.h>

#include "globals.hpp"
#include "lvgl_port.hpp"
#include "main.h"
#include "navio_ui.h"

extern "C" void nav_default_apply_navigation_layout(
    bool heading_above_icon, bool show_directions, bool large_directions);

ChronosESP32 watch("Chronos Navio"); // set the bluetooth name
Preferences prefs;
Navigation nav;

namespace HardcodedSettings {
constexpr int brightness = 100;
constexpr int screen_timeout = 3; // 30 seconds
constexpr int icon_size = 384;
constexpr int show_eta = 0;
constexpr int show_directions = 1;
constexpr int directions_size = 1;
constexpr uint32_t theme_color = 0xFFFFFF;
constexpr bool show_trip_info = true;
constexpr bool heading_above_icon = false;
} // namespace HardcodedSettings

struct DisplaySettings {
  uint8_t brightness = HardcodedSettings::brightness;
  uint8_t timeout = HardcodedSettings::screen_timeout;
  uint16_t icon_size = HardcodedSettings::icon_size;
  bool show_eta = HardcodedSettings::show_eta;
  bool show_directions = HardcodedSettings::show_directions;
  bool large_directions = HardcodedSettings::directions_size;
  uint32_t theme_color = HardcodedSettings::theme_color;
  bool show_trip_info = HardcodedSettings::show_trip_info;
  bool heading_above_icon = HardcodedSettings::heading_above_icon;
};

bool nav_active = false;
uint32_t nav_crc = 0xFFFFFFFF;
lv_image_dsc_t nav_icon_dsc;

ScreenTimeoutState screen_timeout;
DisplaySettings display_settings;

static void screen_activity(uint32_t extra_ms);
void configCallback(Config config, uint32_t a, uint32_t b);

static uint16_t read_u16_be(const uint8_t *data) {
  return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

static uint32_t read_u32_be(const uint8_t *data) {
  return (static_cast<uint32_t>(data[0]) << 24) |
         (static_cast<uint32_t>(data[1]) << 16) |
         (static_cast<uint32_t>(data[2]) << 8) | data[3];
}

static void persist_display_settings() {
  prefs.putUChar("brightness", display_settings.brightness);
  prefs.putUChar("timeout", display_settings.timeout);
  prefs.putUShort("icon_size", display_settings.icon_size);
  prefs.putBool("show_eta", display_settings.show_eta);
  prefs.putBool("show_dirs", display_settings.show_directions);
  prefs.putBool("large_dirs", display_settings.large_directions);
  prefs.putUInt("theme_color", display_settings.theme_color);
  prefs.putBool("show_trip", display_settings.show_trip_info);
  prefs.putBool("heading_top", display_settings.heading_above_icon);
}

static void apply_display_settings() {
  navio_subject_set_screen_brightness(display_settings.brightness);
  navio_subject_set_screen_timeout(display_settings.timeout);
  navio_subject_set_icon_size(display_settings.icon_size);
  navio_subject_set_show_arrival_time(display_settings.show_eta);
  navio_subject_set_show_directions(display_settings.show_directions);
  navio_subject_set_directions_size(display_settings.large_directions);
  navio_subject_set_theme_color(display_settings.theme_color);
  nav_default_apply_navigation_layout(
      display_settings.heading_above_icon, display_settings.show_directions,
      display_settings.large_directions);
}

static void settingsDataCallback(uint8_t *data, int length) {
  // The Chronos library ignores this command after exposing the complete frame here.
  bool version_one = length == 21 && data[5] == 1;
  bool version_two = length == 23 && data[5] == 2;
  bool version_three = length == 19 && data[5] == 3;
  if ((!version_one && !version_two && !version_three) ||
      data[0] != 0xAB || data[1] != 0 ||
      data[2] != length - 3 || data[3] != 0xFE || data[4] != 0x7E) {
    return;
  }

  DisplaySettings incoming = display_settings;
  uint8_t eta_value;
  uint8_t directions_value;
  uint8_t large_directions_value;
  uint8_t show_trip_value = 1;
  uint8_t heading_value = 0;
  if (version_three) {
    incoming.brightness = data[6];
    incoming.timeout = data[7];
    incoming.icon_size = read_u16_be(data + 8);
    eta_value = data[10];
    directions_value = data[11];
    large_directions_value = data[12];
    incoming.theme_color = read_u32_be(data + 13);
    show_trip_value = data[17];
    heading_value = data[18];
    incoming.show_trip_info = show_trip_value != 0;
    incoming.heading_above_icon = heading_value != 0;
  } else {
    incoming.brightness = data[6];
    incoming.timeout = data[9];
    incoming.icon_size = read_u16_be(data + 11);
    eta_value = data[14];
    directions_value = data[15];
    large_directions_value = data[16];
    incoming.theme_color = read_u32_be(data + 17);
    if (version_two) {
      show_trip_value = data[21];
      heading_value = data[22];
      incoming.show_trip_info = show_trip_value != 0;
      incoming.heading_above_icon = heading_value != 0;
    }
  }
  incoming.show_eta = eta_value != 0;
  incoming.show_directions = directions_value != 0;
  incoming.large_directions = large_directions_value != 0;

  if (incoming.brightness > 100 || incoming.timeout > 4 ||
      incoming.icon_size < 256 || incoming.icon_size > 512 ||
      eta_value > 1 || directions_value > 1 ||
      large_directions_value > 1 || incoming.theme_color > 0xFFFFFF ||
      (version_two && (show_trip_value > 1 || heading_value > 1)) ||
      (version_three && (show_trip_value > 1 || heading_value > 1))) {
    Timber.w("Rejected invalid Navio Bridge display settings");
    return;
  }

  display_settings = incoming;
  persist_display_settings();
  apply_display_settings();
  configCallback(CF_NAV_DATA, 0, 0);
  screen_activity(0);
  Timber.i("Applied display settings from Navio Bridge");
}

static void load_display_settings() {
  display_settings.brightness = prefs.getUChar(
      "brightness", HardcodedSettings::brightness);
  display_settings.timeout = prefs.getUChar(
      "timeout", HardcodedSettings::screen_timeout);
  display_settings.icon_size = prefs.getUShort(
      "icon_size", HardcodedSettings::icon_size);
  display_settings.show_eta = prefs.getBool(
      "show_eta", HardcodedSettings::show_eta);
  display_settings.show_directions = prefs.getBool(
      "show_dirs", HardcodedSettings::show_directions);
  display_settings.large_directions = prefs.getBool(
      "large_dirs", HardcodedSettings::directions_size);
  display_settings.theme_color = prefs.getUInt(
      "theme_color", HardcodedSettings::theme_color);
  display_settings.show_trip_info = prefs.getBool(
      "show_trip", HardcodedSettings::show_trip_info);
  display_settings.heading_above_icon = prefs.getBool(
      "heading_top", HardcodedSettings::heading_above_icon);
}

static uint32_t physical_internal_ram_kb() {
#if defined(CONFIG_IDF_TARGET_ESP32)
  return 520;
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
  return 320;
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  return 512;
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
  return 272;
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  return 400;
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  return 512;
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
  return 320;
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
  return 768;
#else
  return 0;
#endif
}

static uint8_t brightness_percent_to_level(int32_t value) {
  value = constrain(value, 0, 100);
  return (uint8_t)((value * 255) / 100);
}

static void set_screen_brightness_level(uint8_t value) {
#if BOARD_HAS_CUSTOM_BRIGHTNESS == 1
  board::set_brightness(value);
#else
  tft.setBrightness(value);
#endif
}

static void apply_screen_brightness() {
  if (!screen_timeout.awake) {
    set_screen_brightness_level(0);
    return;
  }

  set_screen_brightness_level(
      brightness_percent_to_level(navio_subject_get_screen_brightness()));
}

static void set_screen_awake(bool awake) {
  if (screen_timeout.awake == awake) {
    return;
  }

  screen_timeout.awake = awake;
  apply_screen_brightness();
}

static bool screen_is_awake() { return screen_timeout.awake; }

static void screen_activity(uint32_t extra_ms = 0) {
  screen_timeout.last_activity_ms = millis() + extra_ms;
  set_screen_awake(true);
}

static void set_screen_timeout(int32_t value) {
  screen_timeout.enabled = BOARD_ENABLE_SCREEN_TIMEOUT && value < 4;

  if (value <= 0) {
    screen_timeout.timeout_ms = 5000;
  } else if (value < 4) {
    screen_timeout.timeout_ms = 10000UL * (uint32_t)value;
  }

  screen_activity();
}

static void board_screen_input_task() {
  if (board::screen_toggle_requested()) {
    if (screen_is_awake()) {
      set_screen_awake(false);
    } else {
      screen_activity();
    }
    return;
  }

  if (board::wakeup_activity()) {
    screen_activity();
  }
}

static void screen_timeout_task() {
  if (!screen_timeout.enabled) {
    set_screen_awake(true);
    return;
  }

  if (nav.active) {
    screen_activity();
    return;
  }

  uint32_t elapsed = millis() - screen_timeout.last_activity_ms;
  if (screen_timeout.awake && elapsed >= screen_timeout.timeout_ms) {
    Timber.w("Screen timeout");
    set_screen_awake(false);
  }
}



void configCallback(Config config, uint32_t a, uint32_t b) {
  (void)b;
  switch (config) {
  case CF_NAV_DATA: {
    nav = watch.getNavigation();

    Timber.d("Navigation data received: active=%d, title=%s, directions=%s, duration=%s, eta=%s, distance=%s",
             nav.active, nav.title.c_str(), nav.directions.c_str(),
             nav.duration.c_str(), nav.eta.c_str(), nav.distance.c_str());
    String sep = (nav.duration != "" && nav.distance != "") ? " | " : " ";
    if (!nav.active) {
      nav.directions = "nav_start";
      nav.title = "navigation";
      nav.duration = watch.isConnected() ? lv_translation_get("inactive")
                                         : lv_translation_get("disconnected");
      nav.eta = "Chronos";
      nav.distance = "";
      sep = " ";
      nav_crc = 0xFFFFFFFF;
    }
    if (!nav.isNavigation && nav.active) {
      nav.directions = nav.title;
      nav.title = "";
      sep = " ";
    }
    String navText;

    if (nav.active) {
      if (navio_subject_get_show_arrival_time() && nav.eta != "") {
        navText = nav.eta;
      }
      if (display_settings.show_trip_info) {
        String tripText = nav.duration + sep + nav.distance;
        if (navText != "" && tripText != "") {
          navText += "\n";
        }
        navText += tripText;
      }
      if (navText == "") {
        navText = " ";
      }

      
      if (nav.title == "") {
        nav.title = " ";
      }
      if (nav.speed != "") {
        nav.title = nav.title + " | " + nav.speed;
      }
    } else {
      String nl = (nav.duration == "" && nav.distance == "") ? "" : "\n";
      navText = nav.eta + nl + nav.duration + sep + nav.distance;
    }

    navio_subject_set_nav_text(navText.c_str());
    navio_subject_set_nav_title(nav.title.c_str());
    navio_subject_set_nav_directions(nav.directions.c_str());
    if (nav.active && nav.hasIcon) {
      navio_subject_set_nav_icon((void *)&nav_icon_dsc);
    } else {
      navio_subject_set_nav_icon((void *)img_navio_tr_48);
    }
  } break;
  case CF_NAV_ICON:

    if (a == 2) {
      nav = watch.getNavigation();
      if (nav_crc != nav.iconCRC) {
        nav_crc = nav.iconCRC;
        navio_subject_set_nav_icon((void *)&nav_icon_dsc);
      }
    }
    break;
  case CF_APP: {
    String appVersion = "v" + watch.getAppVersion();
    navio_subject_set_chronos_app_version(appVersion.c_str());
    prefs.putString("app_version", appVersion);
  } break;
  case CF_FONT:
    navio_subject_set_theme_color(a);
    prefs.putInt("theme_color", a);
    break;
  case CF_RST:
    prefs.clear();
    ESP.restart();
    break;
  case CF_FIND:
    screen_activity();
    break;
  }
}

static uint8_t normalize_screen_rotation(int32_t rotation) {
  int32_t value = rotation % 4;
  if (value < 0) {
    value += 4;
  }
  return (uint8_t)value;
}

static uint8_t display_rotation_from_ui(int32_t rotation) {
  return normalize_screen_rotation(rotation + BOARD_ROTATION_OFFSET);
}

void navio_subject_screen_brightness_change(int32_t value) {
  (void)value;
  apply_screen_brightness();
}

void navio_subject_screen_rotation_change(int32_t value) {
#if BOARD_ROTATION_LOCKED == 1
  return;
#endif

  if (SCREEN_WIDTH != SCREEN_HEIGHT && value % 2 != 0) {
    return;
  }

#if BOARD_SW_ROTATION == 1
  lv_display_set_rotation(
      lv_display_get_default(),
      lvgl_port_get_rotation(display_rotation_from_ui(value)));
#else
  tft.setRotation(display_rotation_from_ui(value));
  lv_obj_invalidate(lv_screen_active());
#endif
}

void navio_subject_screen_timeout_change(int32_t value) {
  set_screen_timeout(value);
}

void setup() {

  Serial.begin(115200);

  prefs.begin("my-app");

  board::before_display_init();

  tft.init();
  tft.initDMA();
  tft.startWrite();
  tft.fillScreen(0x0000);
  set_screen_brightness_level(255);
  board::after_display_init();

  load_display_settings();
  int brightness = display_settings.brightness;
  int screen_timeout_value = display_settings.timeout;
  String app_version = prefs.getString("app_version", "N/A");

  lvgl_port_set_screen_callbacks(screen_is_awake, screen_activity);
  lvgl_port_init();

  navio_ui_init("");
  board::after_ui_init();

  navio_ui_set_screen(SCREEN_WIDTH, SCREEN_HEIGHT);

  lv_screen_load(screen_launch());

  watch.setConfigurationCallback(configCallback);
  watch.setDataCallback(settingsDataCallback);
  watch.begin();
  watch.setBattery(100);
  watch.set24Hour(false);

  nav_icon_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
  nav_icon_dsc.header.cf = LV_COLOR_FORMAT_A1;
  nav_icon_dsc.header.w = 48;
  nav_icon_dsc.header.h = 48;
  nav_icon_dsc.header.stride = 6;
  nav_icon_dsc.header.reserved_2 = 0;
  nav_icon_dsc.header.flags = 0;
  nav_icon_dsc.data_size = 48 * 48 / 8;
  nav_icon_dsc.data = nav.icon;
  nav_icon_dsc.reserved = NULL;

  navio_subject_set_board_mac(watch.getAddress().c_str());
  char version[16];
  lv_snprintf(version, sizeof(version), "v%d.%d.%d", CS_VERSION_MAJOR,
              CS_VERSION_MINOR, CS_VERSION_PATCH);
  navio_subject_set_chronos_esp_version(version);

  navio_subject_set_board_oem(BOARD_OEM);
  navio_subject_set_board_name(BOARD_NAME);
  navio_subject_set_board_type(ESP.getChipModel());
  uint32_t ram_kb = physical_internal_ram_kb();
  navio_subject_set_board_ram(
      ram_kb > 0 ? (String(ram_kb) + "KB").c_str() : "N/A");
  navio_subject_set_board_flash(
      (String((ESP.getFlashChipSize() / (1024.0 * 1024.0)), 0) + "MB").c_str());
  navio_subject_set_board_psram(
      (String((ESP.getPsramSize() / (1024.0 * 1024.0)), 0) + "MB").c_str());
  navio_subject_set_display_type(DISPLAY_TYPE);

  navio_subject_set_firmware_version(FIRMWARE_VERSION);

  navio_subject_set_screen_mode(UI_MODE);
  navio_subject_set_screen_brightness_supported(!BOARD_HAS_CUSTOM_BRIGHTNESS);

  navio_subject_set_chronos_app_version(app_version.c_str());
  navio_subject_set_language(0);
  int rotation = 0;
#if BOARD_ROTATION_LOCKED == 1 && BOARD_ROTATION_VALUE >= 0
  rotation = BOARD_ROTATION_VALUE;
#endif
  navio_subject_set_screen_rotation(rotation);
  navio_subject_set_screen_brightness(brightness);
  navio_subject_set_screen_timeout(screen_timeout_value);

  navio_subject_set_icon_size(display_settings.icon_size);
  navio_subject_set_directions_size(display_settings.large_directions);
  navio_subject_set_show_system_time(false);
  navio_subject_set_show_arrival_time(display_settings.show_eta);
  navio_subject_set_show_directions(display_settings.show_directions);
  nav_default_apply_navigation_layout(
      display_settings.heading_above_icon, display_settings.show_directions,
      display_settings.large_directions);
  // navio_subject_set_nav_icon((void *)&nav_icon_dsc);
  navio_subject_set_nav_text("Chronos");
  navio_subject_set_nav_title("navigation");
  navio_subject_set_nav_directions("nav_info");
  navio_subject_set_theme_color(display_settings.theme_color);

  Serial.println("Setup complete");
}

void loop() {
  lv_timer_handler(); // Update the UI-
  delay(5);
  watch.loop();
  board::loop();
  board_screen_input_task();

  navio_subject_set_connected(watch.isConnected());
  navio_subject_set_navigation(nav.active);

  screen_timeout_task();
}
