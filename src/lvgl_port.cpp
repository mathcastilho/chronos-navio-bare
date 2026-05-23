#include "lvgl_port.hpp"

#include <Arduino.h>

#include "main.h"

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

#if !USE_DYNAMIC_BUFFERS
static uint8_t lv_buffer[LV_BUFFER_COUNT][LV_BUFFER_SIZE];
#endif

static LvglPortScreenAwakeFn screen_is_awake_cb = nullptr;
static LvglPortScreenActivityFn screen_activity_cb = nullptr;

void lvgl_port_set_screen_callbacks(LvglPortScreenAwakeFn is_awake,
                                    LvglPortScreenActivityFn activity) {
  screen_is_awake_cb = is_awake;
  screen_activity_cb = activity;
}

lv_display_rotation_t lvgl_port_get_rotation(uint8_t rotation) {
  if (rotation > 3) {
    return LV_DISPLAY_ROTATION_0;
  }

  return (lv_display_rotation_t)rotation;
}

static void lvgl_port_set_display_buffers(lv_display_t *display) {
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

static void lvgl_port_flush(lv_display_t *display, const lv_area_t *area,
                            uint8_t *data) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

#if BOARD_SW_ROTATION
  lv_display_rotation_t rotation = lv_display_get_rotation(display);
  lv_area_t rotated_area;
  if (rotation != LV_DISPLAY_ROTATION_0) {
    lv_color_format_t cf = lv_display_get_color_format(display);
    if (cf == LV_COLOR_FORMAT_RGB565_SWAPPED) {
      cf = LV_COLOR_FORMAT_RGB565;
    }

    rotated_area = *area;
    lv_display_rotate_area(display, &rotated_area);

    uint32_t src_stride =
        lv_draw_buf_width_to_stride(lv_area_get_width(area), cf);
    uint32_t dest_stride =
        lv_draw_buf_width_to_stride(lv_area_get_width(&rotated_area), cf);
    static uint8_t rotated_buf[LV_BUFFER_SIZE];
    lv_draw_sw_rotate(data, rotated_buf, w, h, src_stride, dest_stride,
                      rotation, cf);

    area = &rotated_area;
    data = rotated_buf;
  }
#else
  (void)w;
  (void)h;
#endif

  if (tft.getStartCount() == 0) {
    tft.endWrite();
  }

  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1,
                   area->y2 - area->y1 + 1, (uint16_t *)data);
  lv_display_flush_ready(display);
}

#if BOARD_USE_ROUNDER_CB
static void lvgl_port_rounder_event_cb(lv_event_t *e) {
  lv_area_t *area = lv_event_get_invalidated_area(e);
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;
  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}
#endif

#if BOARD_HAS_TOUCH
static bool wake_touch_active = false;

static void lvgl_port_touch_read(lv_indev_t *indev_driver,
                                 lv_indev_data_t *data) {
  (void)indev_driver;

  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched) {
    wake_touch_active = false;
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  bool was_awake = screen_is_awake_cb == nullptr || screen_is_awake_cb();
  if (screen_activity_cb != nullptr) {
    screen_activity_cb(0);
  }

  if (!was_awake || wake_touch_active) {
    wake_touch_active = true;
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = touchX;
  data->point.y = touchY;
}
#endif

static uint32_t my_tick(void) { return millis(); }

#if LV_USE_LOG == 1
void my_print(lv_log_level_t level, const char *buf) {
  // Serial.printf("[LVGL] %s: %s\n", lv_log_level_to_str(level), buf);
  Serial.write(buf, strlen(buf));
}
#endif

lv_display_t *lvgl_port_init(void) {

  lv_init();

  lv_tick_set_cb(my_tick);

#if LV_USE_LOG == 1
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t *display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_flush_cb(display, lvgl_port_flush);
  lvgl_port_set_display_buffers(display);

#if BOARD_USE_ROUNDER_CB
  lv_display_add_event_cb(display, lvgl_port_rounder_event_cb,
                          LV_EVENT_INVALIDATE_AREA, NULL);
#endif

#if BOARD_HAS_TOUCH
  lv_indev_t *input = lv_indev_create();
  lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(input, lvgl_port_touch_read);
#endif

  return display;
}
