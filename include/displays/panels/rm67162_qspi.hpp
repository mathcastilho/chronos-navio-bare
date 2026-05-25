#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>


class RM67162QspiPanel {
public:
  Arduino_GFX *gfx;

  RM67162QspiPanel() {
    static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
        LCD_CS, LCD_SCK, LCD_SD0, LCD_SD1, LCD_SD2, LCD_SD3);

    gfx = new Arduino_RM67162(bus, LCD_RST, 1, false);
  }

  bool init(void) {
#if LCD_EN >= 0
    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);
#endif
    return gfx->begin();
  }

  void initDMA(void) {}
  void fillScreen(uint16_t color) { gfx->fillScreen(color); }
  void setRotation(uint8_t rotation) {}
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h,
                 uint16_t *data) {
    gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
  }
  void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                    uint16_t *data) {
    gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
  }
  void startWrite(void) {}
  uint32_t getStartCount(void) { return 0; }
  void endWrite(void) {}
  void setBrightness(uint8_t brightness) {}
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    gfx->writePixel(x, y, color);
  }
};
