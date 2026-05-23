#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LovyanFT5x06Touch {
  lgfx::Touch_FT5x06 _touch_instance;

public:
  template <typename Panel> void attach(Panel &panel) {
    auto cfg = _touch_instance.config();

    cfg.x_min = 0;
    cfg.x_max = 319;
    cfg.y_min = 0;
    cfg.y_max = 479;
    cfg.pin_int = -1;
    cfg.bus_shared = true;
    cfg.offset_rotation = 0;
    cfg.i2c_port = 0;
    cfg.i2c_addr = 0x38;
    cfg.pin_sda = 38;
    cfg.pin_scl = 39;
    cfg.freq = 400000;

    _touch_instance.config(cfg);
    panel.setTouchDevice(&_touch_instance);
  }

  bool init(void) { return true; }
  void setRotation(uint8_t rotation) {}

  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    return panel.getTouch(x, y);
  }
};
