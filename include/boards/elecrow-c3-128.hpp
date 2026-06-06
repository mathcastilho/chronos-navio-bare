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
#define TP_RST -1

// display
#define TFT_SPI_HOST SPI2_HOST

#define TFT_SCLK 6
#define TFT_MOSI 7
#define TFT_MISO -1
#define TFT_DC 2
#define TFT_CS 10
#define TFT_RST -1

#define TFT_BL -1 // unused (connected on IO extender)
#define VIBRATION_PIN 0 // dummy (connected on IO extender)

#define BUZZER_PIN 3

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2


// ELECROW C3 I2C IO extender
#define PI4IO_I2C_ADDR 0x43

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1


#ifndef BOARD_OEM
#define BOARD_OEM "Elecrow"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "C3 1.28\""
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif
#define BOARD_HAS_TOUCH 1
#define BOARD_HAS_CUSTOM_BRIGHTNESS 1

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/gc9a01_spi.hpp"
#include "displays/touch/lovyan_cst816s.hpp"

#include <Arduino.h>
#include <Wire.h>

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<GC9A01SpiPanel, LovyanCST816STouch>;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *    OTHER FUNCTIONS 
 *********************/
inline void init_io_extender()
{
    Wire.beginTransmission(PI4IO_I2C_ADDR);
    Wire.write(0x01); // test register
    Wire.endTransmission();
    Wire.requestFrom(PI4IO_I2C_ADDR, 1);
    uint8_t rxdata = Wire.read();
    Serial.print("Device ID: ");
    Serial.println(rxdata, HEX);

    Wire.beginTransmission(PI4IO_I2C_ADDR);
    Wire.write(0x03);                                                 // IO direction register
    Wire.write((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)); // set pins 0, 1, 2 as outputs
    Wire.endTransmission();

    Wire.beginTransmission(PI4IO_I2C_ADDR);
    Wire.write(0x07);                                                    // Output Hi-Z register
    Wire.write(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4))); // set pins 0, 1, 2 low
    Wire.endTransmission();
}

inline void set_pin_io(uint8_t pin_number, bool value)
{

    Wire.beginTransmission(PI4IO_I2C_ADDR);
    Wire.write(0x05); // test register
    Wire.endTransmission();
    Wire.requestFrom(PI4IO_I2C_ADDR, 1);
    uint8_t rxdata = Wire.read();
    Serial.print("Before the change: ");
    Serial.println(rxdata, HEX);

    Wire.beginTransmission(PI4IO_I2C_ADDR);
    Wire.write(0x05); // Output register

    if (!value)
        Wire.write((~(1 << pin_number)) & rxdata); // set pin low
    else
        Wire.write((1 << pin_number) | rxdata); // set pin high
    Wire.endTransmission();
}

inline void elecrow_c3_init()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    init_io_extender();
    delay(100);
    set_pin_io(0, false);
    set_pin_io(2, true);
    set_pin_io(3, true);
    set_pin_io(4, true);
}

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) { elecrow_c3_init(); }
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void set_brightness(uint8_t value) { set_pin_io(2, value > 0); }
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) {}
} // namespace board
