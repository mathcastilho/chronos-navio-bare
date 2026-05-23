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

#include "main.h"
#include "navio_ui.h"

#ifndef USE_DYNAMIC_BUFFERS
#define USE_DYNAMIC_BUFFERS 0
#endif

#ifndef LV_BUFFER_COUNT
#define LV_BUFFER_COUNT 2
#endif

#if LV_BUFFER_COUNT != 1 && LV_BUFFER_COUNT != 2
#error "LV_BUFFER_COUNT must be 1 or 2"
#endif

#if USE_DYNAMIC_BUFFERS && !defined(BUFFER_FLAGS)
#define BUFFER_FLAGS MALLOC_CAP_DEFAULT
#endif

#if USE_DYNAMIC_BUFFERS
#include <esp_heap_caps.h>
#endif

ChronosESP32 watch("Chronos Navio"); // set the bluetooth name
Preferences prefs;
Navigation nav;

bool nav_active = false;
uint32_t nav_crc = 0xFFFFFFFF;
lv_image_dsc_t nav_icon_dsc;

ScreenTimeoutState screen_timeout;

#if !USE_DYNAMIC_BUFFERS
uint8_t lv_buffer[LV_BUFFER_COUNT][LV_BUFFER_SIZE];
#endif

static void lv_set_display_buffers(lv_display_t *display) {
  uint8_t *buffer = NULL;
  uint8_t *buffer2 = NULL;

#if USE_DYNAMIC_BUFFERS
  buffer = (uint8_t *)heap_caps_malloc(LV_BUFFER_SIZE, BUFFER_FLAGS);
#if LV_BUFFER_COUNT == 2
  buffer2 = (uint8_t *)heap_caps_malloc(LV_BUFFER_SIZE, BUFFER_FLAGS);
#endif

  if (buffer == NULL || (LV_BUFFER_COUNT == 2 && buffer2 == NULL)) {
    while (true) {
      delay(1000);
    }
  }
#else
  buffer = lv_buffer[0];
#if LV_BUFFER_COUNT == 2
  buffer2 = lv_buffer[1];
#endif
#endif

  lv_display_set_buffers(display, buffer, buffer2, LV_BUFFER_SIZE,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
}

lv_display_rotation_t get_rotation(uint8_t rotation) {
  if (rotation > 3)
    return LV_DISPLAY_ROTATION_0;
  return (lv_display_rotation_t)rotation;
}

/* Display flushing */
void my_disp_flush(lv_display_t *display, const lv_area_t *area,
                   uint8_t *data) {

  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

#ifdef SW_ROTATION
  lv_display_rotation_t rotation = lv_display_get_rotation(display);
  lv_area_t rotated_area;
  if (rotation != LV_DISPLAY_ROTATION_0) {
    lv_color_format_t cf = lv_display_get_color_format(display);
    /*RGB565 swapped does not support rotation, use RGB565 instead*/
    if (cf == LV_COLOR_FORMAT_RGB565_SWAPPED) {
      cf = LV_COLOR_FORMAT_RGB565;
    }
    /*Calculate the position of the rotated area*/
    rotated_area = *area;
    lv_display_rotate_area(display, &rotated_area);
    /*Calculate the source stride (bytes in a line) from the width of the area*/
    uint32_t src_stride =
        lv_draw_buf_width_to_stride(lv_area_get_width(area), cf);
    /*Calculate the stride of the destination (rotated) area too*/
    uint32_t dest_stride =
        lv_draw_buf_width_to_stride(lv_area_get_width(&rotated_area), cf);
    /*Have a buffer to store the rotated area and perform the rotation*/
    static uint8_t rotated_buf[LV_BUFFER_SIZE];
    lv_draw_sw_rotate(data, rotated_buf, w, h, src_stride, dest_stride,
                      rotation, cf);
    /*Use the rotated area and rotated buffer from now on*/
    area = &rotated_area;
    data = rotated_buf;
  }
#endif

  if (tft.getStartCount() == 0) {
    tft.endWrite();
  }

  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1,
                   area->y2 - area->y1 + 1, (uint16_t *)data);
  lv_display_flush_ready(display); /* tell lvgl that flushing is done */
}

void rounder_event_cb(lv_event_t *e) {
  lv_area_t *area = lv_event_get_invalidated_area(e);
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;

  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  // round the start of coordinate down to the nearest 2M number
  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  // round the end of coordinate up to the nearest 2N+1 number
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}

static uint8_t brightness_percent_to_level(int32_t value) {
  value = constrain(value, 0, 100);
  return (uint8_t)((value * 255) / 100);
}

static void apply_screen_brightness() {
  if (!screen_timeout.awake) {
    tft.setBrightness(0);
    return;
  }

  tft.setBrightness(
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
  screen_timeout.enabled = value < 4;

  if (value <= 0) {
    screen_timeout.timeout_ms = 5000;
  } else if (value < 4) {
    screen_timeout.timeout_ms = 10000UL * (uint32_t)value;
  }

  screen_activity();
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

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched) {
    screen_timeout.wake_touch_active = false;
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    bool was_awake = screen_is_awake();
    screen_activity();

    if (!was_awake || screen_timeout.wake_touch_active) {
      screen_timeout.wake_touch_active = true;
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }

    data->state = LV_INDEV_STATE_PRESSED;
    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

static uint32_t my_tick(void) { return millis(); }

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

void navio_subject_screen_brightness_change(int32_t value) {
  apply_screen_brightness();
  prefs.putInt("brightness", value);
}

void navio_subject_screen_rotation_change(int32_t value) {

  if (SCREEN_WIDTH != SCREEN_HEIGHT && value % 2 != 0) {
    return;
  }

#ifdef SW_ROTATION
  lv_display_set_rotation(lv_display_get_default(), get_rotation(value));
#else
  tft.setRotation(value);
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
  prefs.putInt("show_time", value);
}

void navio_subject_show_arrival_time_change(int32_t value) {
  prefs.putInt("show_eta", value);
}
void navio_subject_show_directions_change(int32_t value) {
  prefs.putInt("show_directions", value);
}

void on_reset_confirm_cb(lv_event_t *e) {

  prefs.clear();
  ESP.restart();
}

void on_settings_status(bool state) {}

#if LV_USE_LOG == 1
void my_print(lv_log_level_t level, const char *buf) {
  // Serial.printf("[LVGL] %s: %s\n", lv_log_level_to_str(level), buf);
  Serial.write(buf, strlen(buf));
}
#endif

void setup() {

  Serial.begin(115200);

  prefs.begin("my-app");

#ifdef ELECROW_C3
  elecrow_c3_init();
#endif

  tft.init();
  tft.initDMA();
  tft.startWrite();
  tft.fillScreen(0x0000);
  tft.setBrightness(255);

  int brightness = prefs.getInt("brightness", 80);
  int language = prefs.getInt("language", 0);
  int rotation = prefs.getInt("rotation", 0);
  int screen_timeout = prefs.getInt("screen_timeout", 2);
  bool hr24 = prefs.getBool("hr24", false);
  int icon_size = prefs.getInt("icon_size", 0);

  int show_time = prefs.getInt("show_time", 1);
  int show_eta = prefs.getInt("show_eta", 1);
  int show_directions = prefs.getInt("show_directions", 1);

  String app_version = prefs.getString("app_version", "N/A");

  lv_init();

  lv_tick_set_cb(my_tick);

#if LV_USE_LOG == 1
  lv_log_register_print_cb(my_print);
#endif

  static lv_display_t *lv_display =
      lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_color_format(lv_display, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_flush_cb(lv_display, my_disp_flush);

  lv_set_display_buffers(lv_display);
  lv_display_add_event_cb(lv_display, rounder_event_cb,
                          LV_EVENT_INVALIDATE_AREA, NULL);

  static lv_indev_t *lv_input = lv_indev_create();
  lv_indev_set_type(lv_input, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(lv_input, my_touchpad_read);

  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Hello LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  navio_ui_init("");

  set_screen(SCREEN_WIDTH, SCREEN_HEIGHT);

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
  navio_subject_set_board_ram(
      (String((ESP.getHeapSize() / 1024.0), 0) + "KB").c_str());
  navio_subject_set_board_flash(
      (String((ESP.getFlashChipSize() / (1024.0 * 1024.0)), 0) + "MB").c_str());
  navio_subject_set_board_psram(
      (String((ESP.getPsramSize() / (1024.0 * 1024.0)), 0) + "MB").c_str());
  navio_subject_set_display_type(DISPLAY_TYPE);

  navio_subject_set_firmware_version(FIRMWARE_VERSION);

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

  String time =
      watch.getHourZ() + watch.getTime(":%M ") + watch.getAmPmC(false);
  navio_subject_set_system_time(time.c_str());

  navio_subject_set_connected(watch.isConnected());
  navio_subject_set_navigation(nav.active);

  screen_timeout_task();
}
