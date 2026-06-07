#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 536
#define SCREEN_HEIGHT 240

#define LCD_CS 6
#define LCD_SCK 47
#define LCD_SD0 18
#define LCD_SD1 7
#define LCD_SD2 48
#define LCD_SD3 5
#define LCD_RST 17
#define LCD_EN 1

#define LV_BUFFER_SIZE (0x4000)
#define LV_BUFFER_COUNT 1

#define USE_DYNAMIC_BUFFERS 1

#define BUFFER_FLAGS MALLOC_CAP_SPIRAM

#define BOARD_OEM "HW Media Lab"
#define BOARD_NAME "Webscreen"
#define DISPLAY_TYPE "AMOLED"

#define BOARD_USE_ROUNDER_CB 1

#define UI_MODE 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/rm67162_qspi.hpp"
#include "displays/touch/none.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = display::DisplayWrapper<RM67162QspiPanel, NoTouch>;
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
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) {}
} // namespace board
