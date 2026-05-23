#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define TFT_BLACK 0x00000

#ifndef CO5300_COL_OFFSET
#define CO5300_COL_OFFSET 6
#endif

class CO5300QspiPanel {
public:
  Arduino_GFX *gfx;

  CO5300QspiPanel() {
    static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
        LCD_CS, LCD_SCK, LCD_SD0, LCD_SD1, LCD_SD2, LCD_SD3);

    gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, SCREEN_WIDTH,
                             SCREEN_HEIGHT, CO5300_COL_OFFSET, 0, 0, 0);
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
  void setBrightness(uint8_t brightness) {
    ((Arduino_CO5300 *)gfx)->setBrightness(brightness);
  }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    gfx->writePixel(x, y, color);
  }
};
