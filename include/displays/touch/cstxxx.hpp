#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "TouchDrvCSTXXX.hpp"

#ifndef CSTXXX_I2C_ADDR
#define CSTXXX_I2C_ADDR 0x15
#endif

class CSTXXXTouch {
public:
  TouchDrvCSTXXX touch;

  template <typename Panel> void attach(Panel &panel) {}

  bool init(void) {
    touch.setPins(TOUCH_RST, TOUCH_IRQ);
    touch.begin(Wire, CSTXXX_I2C_ADDR, TOUCH_SDA, TOUCH_SCL);
#ifdef CSTXXX_MAX_X
    touch.setMaxCoordinates(CSTXXX_MAX_X, CSTXXX_MAX_Y);
#endif
#ifdef CSTXXX_MIRROR_X
    touch.setMirrorXY(CSTXXX_MIRROR_X, CSTXXX_MIRROR_Y);
#endif
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
