#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LovyanXPT2046Touch {
  lgfx::Touch_XPT2046 _touch_instance;

public:
  template <typename Panel> void attach(Panel &panel) {
    auto cfg = _touch_instance.config();

    cfg.x_min = 0;
    cfg.x_max = SCREEN_WIDTH;
    cfg.y_min = 0;
    cfg.y_max = SCREEN_HEIGHT;
    cfg.pin_int = TP_INT;
    cfg.pin_rst = TP_RST;
    cfg.bus_shared = true;
    cfg.offset_rotation = 2;
    
    cfg.spi_host = TP_SPI_HOST;
    cfg.pin_sclk = TP_SCLK;
    cfg.pin_mosi = TP_MOSI;
    cfg.pin_miso = TP_MISO;
    cfg.pin_cs = TP_CS;

    _touch_instance.config(cfg);
    panel.setTouchDevice(&_touch_instance);
  }

  bool init(void) { return true; }
  void setRotation(uint8_t rotation) {}

  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    return panel.getTouch(x, y);
  }
};
