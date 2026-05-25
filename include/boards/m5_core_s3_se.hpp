#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS  (MALLOC_CAP_INTERNAL)

#ifndef BOARD_OEM
#define BOARD_OEM "M5Stack"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "Core S3SE"
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif
#define BOARD_HAS_TOUCH 1
#define BOARD_ROTATION_OFFSET 1

#define UI_MODE 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/panels/m5_stack.hpp"

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
inline void after_display_init(void) { tft.setRotation(BOARD_ROTATION_OFFSET); }
inline void after_ui_init(void) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) { M5.update(); }
} // namespace board
