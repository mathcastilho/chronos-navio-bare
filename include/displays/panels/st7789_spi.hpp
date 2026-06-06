#pragma once

#define LGFX_USE_V1
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <LovyanGFX.hpp>

#include "displays/panels/lovyan_light_config.hpp"

class ST7789SpiPanel {
public:
  Arduino_DataBus *bus;
  Arduino_GFX *gfx;
  lgfx::Light_PWM light;

  ST7789SpiPanel() {
    bus = new Arduino_ESP32SPIDMA(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO, TFT_SPI_HOST);
    gfx = new Arduino_ST7789(bus, TFT_RST, 0, false, SCREEN_WIDTH,
                              SCREEN_HEIGHT);

    auto cfg = light.config();
    cfg.pin_bl = TFT_BL;
    cfg.invert = TFT_BL_INVERTED;
    cfg.freq = 44100;
    cfg.pwm_channel = 1;
    light.config(cfg);
  }

  bool init(void) {

#ifdef LCD_IM0
    pinMode(LCD_IM0, OUTPUT);
    digitalWrite(LCD_IM0, LCD_IM0_STATE);
#endif
#ifdef LCD_IM1
    pinMode(LCD_IM1, OUTPUT);
    digitalWrite(LCD_IM1, LCD_IM1_STATE);
#endif

    bool state = gfx->begin();
    light.init(0);
    return state;
  }

  void initDMA(void) {}
  void fillScreen(uint16_t color) { gfx->fillScreen(color); }
  void setRotation(uint8_t rotation) {
    rotation = (rotation + 4) % 8;
    gfx->setRotation(rotation);
    switch (rotation)
    {
    case 1:
        rotation = ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
        break;
    case 2:
        rotation = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
        break;
    case 3:
        rotation = ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
        break;
    case 4:
        rotation = ST7789_MADCTL_MX | ST7789_MADCTL_RGB;
        break;
    case 5:
        rotation = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
        break;
    case 6:
        rotation = ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
        break;
    case 7:
        rotation = ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
        break;
    default: // case 0:
        rotation = ST7789_MADCTL_RGB;
        break;
    }
    bus->beginWrite();
    bus->writeC8D8(ST7789_MADCTL, rotation | 1 << 3);
    bus->endWrite();
  }
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
