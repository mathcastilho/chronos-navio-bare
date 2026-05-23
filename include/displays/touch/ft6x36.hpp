#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "TouchDrvFT6X36.hpp"

#ifndef FT6X36_I2C_ADDR
#define FT6X36_I2C_ADDR 0x38
#endif

class FT6X36Touch {
public:
  TouchDrvFT6X36 touch;

  template <typename Panel> void attach(Panel &panel) {}

  bool init(void) {
    touch.setPins(TOUCH_RST, TOUCH_IRQ);
    touch.begin(Wire, FT6X36_I2C_ADDR, TOUCH_SDA, TOUCH_SCL);
    return true;
  }

  void setRotation(uint8_t rotation) {}

  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    int16_t x_arr[5], y_arr[5];
    uint8_t touched =
        touch.getPoint(x_arr, y_arr, touch.getSupportTouchPoint());
    *x = x_arr[0];
    *y = y_arr[0];
    return touched;
  }
};
