#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LovyanCST816STouch {
  lgfx::Touch_CST816S _touch_instance;

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
    cfg.offset_rotation = 0;
    cfg.i2c_port = 0;
    cfg.i2c_addr = 0x15;
    cfg.pin_sda = I2C_SDA;
    cfg.pin_scl = I2C_SCL;
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
