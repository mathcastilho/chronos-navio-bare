#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "TouchDrvCSTXXX.hpp"

#ifndef CST816_I2C_ADDR
#define CST816_I2C_ADDR 0x15
#endif

#ifndef TOUCH_ROTATION_OFFSET
#define TOUCH_ROTATION_OFFSET 0
#endif

class CST816Touch {
public:
  TouchDrvCST816 touch;
  uint8_t rotation = TOUCH_ROTATION_OFFSET;

  template <typename Panel> void attach(Panel &panel) {}

  bool init(void) {
    touch.setPins(TOUCH_RST, TOUCH_IRQ);
    touch.begin(Wire, CST816_I2C_ADDR, TOUCH_SDA, TOUCH_SCL);
    return true;
  }

  void setRotation(uint8_t value) {
    rotation = (value + TOUCH_ROTATION_OFFSET) % 4;
  }

  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    int16_t x_arr[5], y_arr[5];
    uint8_t touched =
        touch.getPoint(x_arr, y_arr, touch.getSupportTouchPoint());
    if (!touched) {
      return false;
    }

    int16_t raw_x = x_arr[0];
    int16_t raw_y = y_arr[0];

    switch (rotation) {
    case 1:
      *x = SCREEN_HEIGHT - 1 - raw_y;
      *y = raw_x;
      break;
    case 2:
      *x = SCREEN_WIDTH - 1 - raw_x;
      *y = SCREEN_HEIGHT - 1 - raw_y;
      break;
    case 3:
      *x = raw_y;
      *y = SCREEN_WIDTH - 1 - raw_x;
      break;
    default:
      *x = raw_x;
      *y = raw_y;
      break;
    }

    return true;
  }
};
