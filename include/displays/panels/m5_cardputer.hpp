#pragma once

#include <Arduino.h>
#include <lvgl.h>

#define M5GFX_USING_REAL_LVGL 1
#include "M5Cardputer.h"

class M5Display {
public:
  bool init(void) {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    return true;
  }

  void initDMA(void) {}
  void fillScreen(uint16_t color) { M5Cardputer.Display.fillScreen(color); }
  void setRotation(uint8_t rotation) { M5Cardputer.Display.setRotation(rotation); }
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h,
                 uint16_t *data) {
    M5Cardputer.Display.pushImage(x, y, w, h, data);
  }
  void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                    uint16_t *data) {
    M5Cardputer.Display.pushImageDMA(x, y, w, h, data);
  }
  void startWrite(void) { M5Cardputer.Display.startWrite(); }
  uint32_t getStartCount(void) { return M5Cardputer.Display.getStartCount(); }
  void endWrite(void) { M5Cardputer.Display.endWrite(); }
  void setBrightness(uint8_t brightness) {
    M5Cardputer.Display.setBrightness(brightness);
  }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    M5Cardputer.Display.writePixel(x, y, color);
  }
  bool getTouch(uint16_t *x, uint16_t *y) {
    return M5Cardputer.Display.getTouch(x, y);
  }
};
