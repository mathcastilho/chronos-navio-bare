#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
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

#define BUZZER_PIN 3

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
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

#define MAX_FILE_OPEN 10

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

/*********************
 *      INCLUDES
 *********************/
#include "displays/panels/m5_dial.hpp"

/*********************
 *      TYPEDEFS
 *********************/
static M5DialDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void loop(void) { M5Dial.update(); }
} // namespace board
