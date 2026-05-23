#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 48
#define LCD_SD0 13
#define LCD_SD1 47
#define LCD_SD2 21
#define LCD_SD3 14
#define LCD_RST 11
#define LCD_EN -1 // 17

#define TOUCH_SDA 17
#define TOUCH_SCL 18
#define TOUCH_RST 10
#define TOUCH_IRQ 9

#define BUTTON_HOME 0


#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Viewe"
#define BOARD_NAME "S3 1.5\""
#define DISPLAY_TYPE "AMOLED"

#define BOARD_SW_ROTATION 1
#define BOARD_USE_ROUNDER_CB 1
#define CO5300_COL_OFFSET 6
#define CSTXXX_I2C_ADDR 0x15

#ifndef OFFSET_X
#define OFFSET_X -1
#endif
#ifndef OFFSET_Y
#define OFFSET_Y -1
#endif
#ifndef RGB_ORDER
#define RGB_ORDER false
#endif
#ifndef VIBRATION_PIN
#define VIBRATION_PIN -1
#endif
#ifndef BUZZER_PIN
#define BUZZER_PIN -1
#endif
#ifndef ENCODER_A
#define ENCODER_A -1
#endif
#ifndef ENCODER_B
#define ENCODER_B -1
#endif

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
