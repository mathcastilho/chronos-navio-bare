#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 39
#define LCD_SCK 40
#define LCD_SD0 41
#define LCD_SD1 42
#define LCD_SD2 46
#define LCD_SD3 45
#define LCD_RST -1
#define LCD_EN -1

#define TOUCH_SDA 47
#define TOUCH_SCL 48
#define TOUCH_RST -1
#define TOUCH_IRQ 13
#define CST816_DIRECT_READ 1
#define CST816_RELEASE_GRACE_MS 0


#define LV_BUFFER_SIZE (SCREEN_WIDTH * 200)
#define LV_BUFFER_COUNT 2

#define USE_DYNAMIC_BUFFERS 1

#define BUFFER_FLAGS MALLOC_CAP_SPIRAM

#define BOARD_OEM "M5Stack"
#define BOARD_NAME "StopWatch"
#define DISPLAY_TYPE "AMOLED"
#define BOARD_HAS_TOUCH 1

#define BOARD_SW_ROTATION 1
#define BOARD_USE_ROUNDER_CB 1


#define IOE_MUX_EN   M5IOE1_PIN_1
#define IOE_AU_EN    M5IOE1_PIN_3
#define IOE_MT_PWM   M5IOE1_PIN_9
#define IOE_L3B_EN   M5IOE1_PIN_8
#define IOE_SPK_EN   M5IOE1_PIN_10
#define IOE_TP_EN    M5IOE1_PIN_4
#define IOE_OLED_RST M5IOE1_PIN_5

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/co5300_qspi.hpp"
#include "displays/touch/cst816.hpp"

#include <M5Unified.h>
#include <Wire.h>
#include <M5PM1.h>
#include <M5IOE1.h>

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay = display::DisplayWrapper<CO5300QspiPanel, CST816Touch>;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {
    // M5.begin();
    M5PM1 pm1;
    M5IOE1 ioe1;
    m5pm1_err_t pm1_err = pm1.begin(&Wire, M5PM1_DEFAULT_ADDR, 47, 48, M5PM1_I2C_FREQ_100K);

    if (pm1_err == M5PM1_OK) {
        Serial.println("PM1 initialization successful");
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", pm1_err);
    }
    pm1.setSingleResetDisable(false);

    // Initialize IOE1
    m5ioe1_err_t ioe1_err = ioe1.begin(&Wire, M5IOE1_DEFAULT_ADDR, 47, 48);

    if (ioe1_err == M5IOE1_OK) {
        Serial.println("IOE1 initialization successful");
    } else {
        Serial.printf("IOE1 initialization failed, error code: %d\n", ioe1_err);
    }
    ioe1.pinMode(IOE_MUX_EN, OUTPUT);
    ioe1.pinMode(IOE_AU_EN, OUTPUT);
    ioe1.pinMode(IOE_MT_PWM, OUTPUT);
    ioe1.pinMode(IOE_L3B_EN, OUTPUT);
    ioe1.pinMode(IOE_SPK_EN, OUTPUT);
    ioe1.pinMode(IOE_TP_EN, OUTPUT);
    ioe1.pinMode(IOE_OLED_RST, OUTPUT);

    ioe1.digitalWrite(IOE_L3B_EN, 1);
    ioe1.digitalWrite(IOE_TP_EN, 1);
    ioe1.digitalWrite(IOE_OLED_RST, 1);
}
inline void after_display_init(void) {
    
}
inline void after_ui_init(void) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) { }
} // namespace board