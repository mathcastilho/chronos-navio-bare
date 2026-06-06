#pragma once

/*********************
 *      DEFINES
 *********************/
// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// touch
#define TOUCH_SDA 1
#define TOUCH_SCL 3
#define TOUCH_RST 2
#define TOUCH_IRQ 4

#define CST816_I2C_ADDR 0x2E

// display
#define TFT_SPI_HOST SPI2_HOST

#define TFT_SCLK 40
#define TFT_MOSI 45
#define TFT_MISO -1
#define TFT_DC 41
#define TFT_CS 42
#define TFT_RST 39

#define TFT_BL 13

#define BUZZER_PIN 38

#define LCD_IM0 47
#define LCD_IM1 48
#define LCD_IM0_STATE LOW
#define LCD_IM1_STATE HIGH

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Viewe"
#define BOARD_NAME "S3 2.8\""
#define DISPLAY_TYPE "LCD"
#define BOARD_HAS_TOUCH 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/st7789_spi.hpp"
#include "displays/touch/cst816.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<ST7789SpiPanel, CST816Touch>;
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
