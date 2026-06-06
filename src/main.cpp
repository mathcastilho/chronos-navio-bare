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

ChronosESP32 watch("Chronos Navio"); // set the bluetooth name
Preferences prefs;
Navigation nav;

bool nav_active = false;
uint32_t nav_crc = 0xFFFFFFFF;
lv_image_dsc_t nav_icon_dsc;

ScreenTimeoutState screen_timeout;

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
  switch (config) {
  case CF_NAV_DATA: {
    nav = watch.getNavigation();
    String sep = " | ";
    if (!nav.active) {
      nav.directions = lv_translation_get("nav_start");
      nav.title = lv_translation_get("navigation");
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
      if (navio_subject_get_show_arrival_time()) {
        navText = nav.eta + "\n" + nav.duration + sep + nav.distance;
      } else {
        navText = nav.duration + sep + nav.distance;
      }
    } else {
      navText = nav.eta + "\n" + nav.duration + sep + nav.distance;
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
  case CF_LANG:
    // state not saved internally
    Serial.print("Language: ");
    Serial.println(b);
    break;
  case CF_HR24:
    prefs.putBool("hr24", b);
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
  apply_screen_brightness();
  prefs.putInt("brightness", value);
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
  // screen rotation has changed, invalidate to redraw
  lv_obj_invalidate(lv_screen_active());
#endif

  prefs.putInt("rotation", value);
}

void navio_subject_language_change(int32_t value) {
  // handle language change if needed
  prefs.putInt("language", value);
}

void navio_subject_screen_timeout_change(int32_t value) {
  set_screen_timeout(value);
  prefs.putInt("screen_timeout", value);
}
void navio_subject_icon_size_change(int32_t value) {
  prefs.putInt("icon_size", value);
}

void navio_subject_show_system_time_change(int32_t value) {

  if (!nav.active && !value) {
    navio_subject_set_nav_text("Chronos");
  }
  prefs.putInt("show_time", value);
}

void navio_subject_show_arrival_time_change(int32_t value) {
  prefs.putInt("show_eta", value);
}
void navio_subject_show_directions_change(int32_t value) {
  prefs.putInt("show_directions", value);
}

void navio_ui_reset_confirm_cb(lv_event_t *e) {

  prefs.clear();
  ESP.restart();
}

void on_settings_status(bool state) {}


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

  int brightness = prefs.getInt("brightness", 80);
  int language = prefs.getInt("language", 0);
  int rotation = prefs.getInt("rotation", 0);
#if BOARD_ROTATION_LOCKED == 1 && BOARD_ROTATION_VALUE >= 0
  rotation = BOARD_ROTATION_VALUE;
#endif
  int screen_timeout = prefs.getInt("screen_timeout", 2);
  bool hr24 = prefs.getBool("hr24", false);
  int icon_size = prefs.getInt("icon_size", 0);

  int show_time = prefs.getInt("show_time", 1);
  int show_eta = prefs.getInt("show_eta", 1);
  int show_directions = prefs.getInt("show_directions", 1);

  String app_version = prefs.getString("app_version", "N/A");

  lvgl_port_set_screen_callbacks(screen_is_awake, screen_activity);
  lvgl_port_init();

  navio_ui_init("");
  board::after_ui_init();

  navio_ui_set_screen(SCREEN_WIDTH, SCREEN_HEIGHT);

  lv_screen_load(screen_launch());

  watch.setConfigurationCallback(configCallback);
  watch.begin();
  watch.setBattery(100);
  watch.set24Hour(hr24);

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
  lv_snprintf(version, sizeof(version), "v%d.%d.%d", CHRONOSESP_VERSION_MAJOR,
              CHRONOSESP_VERSION_MINOR, CHRONOSESP_VERSION_PATCH);
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
  navio_subject_set_language(language);
  navio_subject_set_screen_rotation(rotation);
  navio_subject_set_screen_brightness(brightness);
  navio_subject_set_screen_timeout(screen_timeout);

  if (icon_size != 0) {
    navio_subject_set_icon_size(icon_size);
  }
  navio_subject_set_show_system_time(show_time);
  navio_subject_set_show_arrival_time(show_eta);
  navio_subject_set_show_directions(show_directions);
  // navio_subject_set_nav_icon((void *)&nav_icon_dsc);
  navio_subject_set_nav_text("Chronos");
  navio_subject_set_nav_title(lv_translation_get("navigation"));
  navio_subject_set_nav_directions(lv_translation_get("nav_info"));

  Serial.println("Setup complete");
}

void loop() {
  lv_timer_handler(); // Update the UI-
  delay(5);
  watch.loop();
  board::loop();
  board_screen_input_task();

  String time =
      watch.getHourZ() + watch.getTime(":%M ") + watch.getAmPmC(false);
  navio_subject_set_system_time(time.c_str());

  if (!nav.active && navio_subject_get_show_system_time()) {
    navio_subject_set_nav_text(time.c_str());
  }

  navio_subject_set_connected(watch.isConnected());
  navio_subject_set_navigation(nav.active);

  screen_timeout_task();
}
