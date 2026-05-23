#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2

#ifndef BOARD_OEM
#define BOARD_OEM "Elecrow"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "3.5\""
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/ili9488_parallel16.hpp"
#include "displays/touch/lovyan_ft5x06.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<ILI9488Parallel16Panel, LovyanFT5x06Touch>;
static BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void loop(void) {}
} // namespace board
