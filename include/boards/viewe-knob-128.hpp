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


// display
#define TFT_SPI_HOST SPI2_HOST

#define TFT_SCLK 1
#define TFT_MOSI 0
#define TFT_MISO -1
#define TFT_DC 4
#define TFT_CS 10
#define TFT_RST -1

#define TFT_BL 8
#define TFT_BL_INVERTED true


#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1


#ifndef BOARD_OEM
#define BOARD_OEM "Viewe"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "Knob 1.28\""
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif
#define BOARD_HAS_TOUCH 0
#define BOARD_HAS_WAKE_INPUT 1
#define WAKE_BUTTON_PIN 9

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/gc9a01_spi.hpp"
#include "displays/touch/none.hpp"

#include "Button2.h"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<GC9A01SpiPanel, NoTouch>;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *    OTHER FUNCTIONS 
 *********************/

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline Button2 &button(void) {
    static Button2 instance;
    return instance;
}

inline void before_display_init(void) {
    button().begin(WAKE_BUTTON_PIN);
}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void set_brightness(uint8_t value) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) {
    if (button().wasPressed()) {
        button().read();
        return true;
    }
    return false;
 }
inline void loop(void) {
    button().loop();
}
} // namespace board
