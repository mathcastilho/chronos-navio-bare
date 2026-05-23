#pragma once

/*********************
 *      DEFINES
 *********************/
// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST 1

// display
#define TFT_SPI_HOST SPI2_HOST

#define TFT_SCLK 6
#define TFT_MOSI 7
#define TFT_MISO -1
#define TFT_DC 2
#define TFT_CS 10
#define TFT_RST -1

#define TFT_BL 3

#define VIBRATION_PIN -1

#define BUZZER_PIN -1

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2

#define LCD_CS -1
#define LCD_SCK -1
#define LCD_SD0 -1
#define LCD_SD1 -1
#define LCD_SD2 -1
#define LCD_SD3 -1
#define LCD_RST -1
#define LCD_EN -1

#define TOUCH_SDA -1
#define TOUCH_SCL -1
#define TOUCH_RST -1
#define TOUCH_IRQ -1

#define ENCODER_A -1
#define ENCODER_B -1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Guition"
#define BOARD_NAME "C3 1.28\""
#define DISPLAY_TYPE "LCD"

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/gc9a01_spi.hpp"
#include "displays/touch/lovyan_cst816s.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<GC9A01SpiPanel, LovyanCST816STouch>;
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
