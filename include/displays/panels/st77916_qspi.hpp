#pragma once

#define LGFX_USE_V1
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <LovyanGFX.hpp>

#include "displays/panels/lovyan_light_config.hpp"

class ST77916QspiPanel {
public:
  Arduino_DataBus *bus;
  Arduino_GFX *gfx;
  lgfx::Light_PWM light;

  ST77916QspiPanel() {
    bus = new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_SD0, LCD_SD1, LCD_SD2,
                                LCD_SD3);
    gfx = new Arduino_ST77916(bus, LCD_RST, 0, true, SCREEN_WIDTH,
                              SCREEN_HEIGHT);

    auto cfg = light.config();
    cfg.pin_bl = LCD_BL;
    cfg.invert = TFT_BL_INVERTED;
    cfg.freq = 44100;
    cfg.pwm_channel = 1;
    light.config(cfg);
  }

  bool init(void) {
#ifdef LCD_EN
    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);
#endif

    bool state = gfx->begin();
    reInit();
    light.init(0);
    return state;
  }

  void reInit(void) {
    if (LCD_RST != GFX_NOT_DEFINED) {
      pinMode(LCD_RST, OUTPUT);
      digitalWrite(LCD_RST, HIGH);
      delay(100);
      digitalWrite(LCD_RST, LOW);
      delay(ST77916_RST_DELAY);
      digitalWrite(LCD_RST, HIGH);
      delay(ST77916_RST_DELAY);
    } else {
      bus->sendCommand(ST77916_SWRESET);
      delay(ST77916_RST_DELAY);
    }

    bus->batchOperation(st77916_init, sizeof(st77916_init));
    gfx->invertDisplay(false);
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
  void setBrightness(uint8_t brightness) { light.setBrightness(brightness); }
  void writePixel(int32_t x, int32_t y, const uint16_t color) {
    gfx->writePixel(x, y, color);
  }
};
