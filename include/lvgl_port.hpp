#pragma once

#include <lvgl.h>
#include <stdint.h>

typedef bool (*LvglPortScreenAwakeFn)(void);
typedef void (*LvglPortScreenActivityFn)(uint32_t extra_ms);

void lvgl_port_set_screen_callbacks(LvglPortScreenAwakeFn is_awake,
                                    LvglPortScreenActivityFn activity);
void lvgl_port_init(void);
lv_display_rotation_t lvgl_port_get_rotation(uint8_t rotation);
