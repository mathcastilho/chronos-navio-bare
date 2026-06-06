#pragma once

/*********************
 *      DEFINES
 *********************/
// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define TP_SPI_HOST VSPI_HOST
#define TP_SCLK 25
#define TP_MOSI 32
#define TP_MISO 39
#define TP_CS 33
#define TP_INT 36
#define TP_RST -1


// display
#define TFT_SPI_HOST HSPI_HOST

#define TFT_SCLK 14
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_DC 2
#define TFT_CS 15
#define TFT_RST -1

#define TFT_BL 21

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2


#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Guition"
#define BOARD_NAME "2432S028"
#define DISPLAY_TYPE "LCD"
#define BOARD_HAS_TOUCH 1

/*********************
 *      INCLUDES
 *********************/
#include "globals.hpp"
#include "displays/display_wrapper.hpp"
#include "displays/panels/ili9341_spi.hpp"
#include "displays/touch/lovyan_xpt2046.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<ILI9341SpiPanel, LovyanXPT2046Touch>;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {
    uint16_t calib[8];
    bool calibLoaded = prefs.getBytes("touch_calib", calib, sizeof(calib)) == sizeof(calib);
    if (calibLoaded)
    {
        for (int i = 0; i < 8; i++)
        {
            Serial.printf("Stored calib %d: %d\n", i, calib[i]);
        }
        tft.panel.setTouchCalibrate(calib);
    }
    else
    {
        tft.panel.setTextColor(0xFFFFFFU, 0);
        tft.panel.setTextWrap(true, true);
        tft.panel.println("Touch calibration required!\nPress center to start then \ntouch the corners as instructed.\nUse a pen or stylus.");
        uint16_t touchX, touchY;
        while(!tft.getTouch(&touchX, &touchY)) {
            delay(100);
        }

        tft.fillScreen(0x0000);

        Serial.println("No touch calibration found, starting calibration");
        tft.panel.calibrateTouch(calib, 0xFFFF, 0x0000);
        for (int i = 0; i < 8; i++)
        {
            Serial.printf("Configured calib %d: %d\n", i, calib[i]);
        }
        prefs.putBytes("touch_calib", calib, sizeof(calib));
    }
}
inline void after_ui_init(void) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) {}
} // namespace board
