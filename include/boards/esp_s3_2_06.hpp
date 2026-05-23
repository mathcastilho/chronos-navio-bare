#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 410
#define SCREEN_HEIGHT 502
#define OFFSET_X -1
#define OFFSET_Y -1
#define RGB_ORDER false

#define I2C_SDA -1
#define I2C_SCL -1
#define TP_INT -1
#define TP_RST -1

#define TFT_SPI_HOST -1

#define TFT_SCLK -1
#define TFT_MOSI -1
#define TFT_MISO -1
#define TFT_DC -1
#define TFT_CS -1
#define TFT_RST -1
#define TFT_BL -1

#define VIBRATION_PIN -1

#define BUZZER_PIN -1

#define LCD_CS 12
#define LCD_SCK 11
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 8
#define LCD_EN -1

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 9
#define TOUCH_IRQ 38

#define ENCODER_A -1
#define ENCODER_B -1

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 200)
#define LV_BUFFER_COUNT 1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 1

#define BUFFER_FLAGS MALLOC_CAP_SPIRAM

#define BOARD_OEM "Waveshare"
#define BOARD_NAME "S3 2.06\""
#define DISPLAY_TYPE "AMOLED"

#define BOARD_SW_ROTATION 1
#define BOARD_USE_ROUNDER_CB 1
#define CO5300_COL_OFFSET 22
#define FT6X36_I2C_ADDR 0x38

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/co5300_qspi.hpp"
#include "displays/touch/ft6x36.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = display::DisplayWrapper<CO5300QspiPanel, FT6X36Touch>;
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
