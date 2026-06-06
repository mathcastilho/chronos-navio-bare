#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define BUZZER_PIN 3

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1


#ifndef BOARD_OEM
#define BOARD_OEM "M5Stack"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "Dial"
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif
#define BOARD_HAS_TOUCH 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/panels/m5_dial.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = M5DialDisplay;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return M5Dial.BtnA.wasPressed(); }
inline void loop(void) { M5Dial.update(); }
} // namespace board
