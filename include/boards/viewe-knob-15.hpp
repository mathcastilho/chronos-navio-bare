#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466


#define LCD_CS 12
#define LCD_SCK 10
#define LCD_SD0 13
#define LCD_SD1 11
#define LCD_SD2 14
#define LCD_SD3 9
#define LCD_RST 8
#define LCD_EN 17

#define TOUCH_SDA 1
#define TOUCH_SCL 3
#define TOUCH_RST 2
#define TOUCH_IRQ 4

#define ENCODER_A 6
#define ENCODER_B 5

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 2

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_SW_ROTATION 1
#define BOARD_USE_ROUNDER_CB 1
#define CO5300_COL_OFFSET 6
#define CSTXXX_I2C_ADDR 0x15


#ifndef BOARD_OEM
#define BOARD_OEM "Viewe"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "Touch Knob 1.5\""
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "AMOLED"
#endif
#define BOARD_HAS_TOUCH 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/co5300_qspi.hpp"
#include "displays/touch/cstxxx.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = display::DisplayWrapper<CO5300QspiPanel, CSTXXXTouch>;
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
