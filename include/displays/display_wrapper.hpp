#pragma once

#include <Arduino.h>

namespace display {

template <typename Panel, typename Touch> class DisplayWrapper {
public:
  Panel panel;
  Touch touch;

  DisplayWrapper() { touch.attach(panel); }

  bool init(void) {
    bool state = panel.init();
    touch.init();
    return state;
  }

  void initDMA(void) { panel.initDMA(); }
  void fillScreen(uint16_t color) { panel.fillScreen(color); }
  void setRotation(uint8_t rotation) {
    panel.setRotation(rotation);
    touch.setRotation(rotation);
  }
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data) {
    panel.pushImage(x, y, w, h, data);
  }
  void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                    uint16_t *data) {
    panel.pushImageDMA(x, y, w, h, data);
  }
  void startWrite(void) { panel.startWrite(); }
  uint32_t getStartCount(void) { return panel.getStartCount(); }
  void endWrite(void) { panel.endWrite(); }
  void setBrightness(uint8_t brightness) { panel.setBrightness(brightness); }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    panel.writePixel(x, y, color);
  }
  bool getTouch(uint16_t *x, uint16_t *y) { return touch.read(panel, x, y); }
};

} // namespace display
