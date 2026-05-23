#pragma once

#include <Arduino.h>
#include <lvgl.h>

#define M5GFX_USING_REAL_LVGL 1
#include <M5Dial.h>

class M5DialDisplay {
public:
  bool init(void) {
    M5Dial.begin();
    return true;
  }

  void initDMA(void) {}
  void fillScreen(uint16_t color) { M5Dial.Display.fillScreen(color); }
  void setRotation(uint8_t rotation) { M5Dial.Display.setRotation(rotation); }
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h,
                 uint16_t *data) {
    M5Dial.Display.pushImage(x, y, w, h, data);
  }
  void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                    uint16_t *data) {
    M5Dial.Display.pushImageDMA(x, y, w, h, data);
  }
  void startWrite(void) { M5Dial.Display.startWrite(); }
  uint32_t getStartCount(void) { return M5Dial.Display.getStartCount(); }
  void endWrite(void) { M5Dial.Display.endWrite(); }
  void setBrightness(uint8_t brightness) {
    M5Dial.Display.setBrightness(brightness);
  }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    M5Dial.Display.writePixel(x, y, color);
  }
  bool getTouch(uint16_t *x, uint16_t *y) {
    return M5Dial.Display.getTouch(x, y);
  }
};
