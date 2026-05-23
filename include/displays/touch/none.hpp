#pragma once

#include <Arduino.h>

class NoTouch {
public:
  template <typename Panel> void attach(Panel &panel) {}
  bool init(void) { return true; }
  void setRotation(uint8_t rotation) {}
  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    return false;
  }
};
