#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS  (MALLOC_CAP_INTERNAL)

#ifndef BOARD_OEM
#define BOARD_OEM "M5Stack"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "Cardputer"
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif
#define BOARD_HAS_TOUCH 0
#define BOARD_HAS_WAKE_INPUT 1
#define BOARD_ROTATION_LOCKED 1
#define BOARD_ROTATION_VALUE 1

#define BOARD_USE_ROUNDER_CB 1

#define UI_MODE 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/panels/m5_cardputer.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = M5Display;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) { tft.setRotation(BOARD_ROTATION_VALUE); }
inline void after_ui_init(void) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { 
    return M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isKeyPressed('`'); 
}
inline void loop(void) { M5Cardputer.update(); }
} // namespace board
