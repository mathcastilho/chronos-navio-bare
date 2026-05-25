#pragma once

#include <Arduino.h>
#include <lvgl.h>

#define M5GFX_USING_REAL_LVGL 1
#include <M5Unified.h>

class M5Display {
public:
  bool init(void) {
    auto cfg = M5.config();
    M5.begin(cfg);
    return true;
  }

  void initDMA(void) {}
  void fillScreen(uint16_t color) { M5.Display.fillScreen(color); }
  void setRotation(uint8_t rotation) { M5.Display.setRotation(rotation); }
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h,
                 uint16_t *data) {
    M5.Display.pushImage(x, y, w, h, data);
  }
  void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                    uint16_t *data) {
    M5.Display.pushImageDMA(x, y, w, h, data);
  }
  void startWrite(void) { M5.Display.startWrite(); }
  uint32_t getStartCount(void) { return M5.Display.getStartCount(); }
  void endWrite(void) { M5.Display.endWrite(); }
  void setBrightness(uint8_t brightness) {
    M5.Display.setBrightness(brightness);
  }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    M5.Display.writePixel(x, y, color);
  }
  bool getTouch(uint16_t *x, uint16_t *y) {
    return M5.Display.getTouch(x, y);
  }
};
