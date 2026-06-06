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


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Guition"
#define BOARD_NAME "2424S012"
#define DISPLAY_TYPE "LCD"
#define BOARD_HAS_TOUCH 1

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
