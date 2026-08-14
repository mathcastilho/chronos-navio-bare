#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "TouchDrvCSTXXX.hpp"

#ifndef CST816_I2C_ADDR
#define CST816_I2C_ADDR 0x15
#endif

#ifndef TOUCH_ROTATION_OFFSET
#define TOUCH_ROTATION_OFFSET 0
#endif

#ifndef CST816_RAW_MAX_X
#define CST816_RAW_MAX_X SCREEN_WIDTH
#endif

#ifndef CST816_RAW_MAX_Y
#define CST816_RAW_MAX_Y SCREEN_HEIGHT
#endif

#ifndef CST816_RELEASE_GRACE_MS
#define CST816_RELEASE_GRACE_MS 80
#endif

#ifndef CST816_DIRECT_READ
#define CST816_DIRECT_READ 0
#endif

class CST816Touch {
public:
  TouchDrvCST816 touch;
  uint8_t rotation = TOUCH_ROTATION_OFFSET;
  bool last_touched = false;
  uint32_t last_touch_ms = 0;
  int16_t last_raw_x = 0;
  int16_t last_raw_y = 0;

  template <typename Panel> void attach(Panel &panel) {}

  bool init(void) {
    touch.setPins(TOUCH_RST, TOUCH_IRQ);
    touch.begin(Wire, CST816_I2C_ADDR, TOUCH_SDA, TOUCH_SCL);
    configureInterruptMode();
    return true;
  }

  void setRotation(uint8_t value) {
    rotation = (value + TOUCH_ROTATION_OFFSET) % 4;
  }

  template <typename Panel> bool read(Panel &panel, uint16_t *x, uint16_t *y) {
    int16_t raw_x = 0;
    int16_t raw_y = 0;
    bool touched = readRawPoint(&raw_x, &raw_y);

    if (!touched && last_touched &&
        millis() - last_touch_ms <= CST816_RELEASE_GRACE_MS) {
      raw_x = last_raw_x;
      raw_y = last_raw_y;
      touched = true;
    }

    if (!touched) {
      last_touched = false;
      return false;
    }

    last_touched = true;
    last_touch_ms = millis();
    last_raw_x = raw_x;
    last_raw_y = raw_y;

    int32_t scaled_x = constrain((int32_t)raw_x * (SCREEN_WIDTH - 1) /
                                     CST816_RAW_MAX_X,
                                 0, SCREEN_WIDTH - 1);
    int32_t scaled_y = constrain((int32_t)raw_y * (SCREEN_HEIGHT - 1) /
                                     CST816_RAW_MAX_Y,
                                 0, SCREEN_HEIGHT - 1);

    switch (rotation) {
    case 1:
      *x = SCREEN_HEIGHT - 1 - scaled_y;
      *y = scaled_x;
      break;
    case 2:
      *x = SCREEN_WIDTH - 1 - scaled_x;
      *y = SCREEN_HEIGHT - 1 - scaled_y;
      break;
    case 3:
      *x = scaled_y;
      *y = SCREEN_WIDTH - 1 - scaled_x;
      break;
    default:
      *x = scaled_x;
      *y = scaled_y;
      break;
    }

    return true;
  }

private:
  void writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
  }

  void configureInterruptMode() {
#if TOUCH_IRQ >= 0
    pinMode(TOUCH_IRQ, INPUT_PULLUP);
#endif
    writeRegister(0xFA, 0x20);
    writeRegister(0xED, 20);
  }

  bool readRawPoint(int16_t *x, int16_t *y) {
#if CST816_DIRECT_READ
    uint8_t data[6];
    for (uint8_t retry = 0; retry < 3; retry++) {
      if (!readRegister(0x02, data, sizeof(data))) {
        return false;
      }

      uint8_t touched = data[0] & 0x0F;
      if (touched == 0) {
        return false;
      }
      if (touched == 1) {
        *x = data[2] | ((data[1] & 0x0F) << 8);
        *y = data[4] | ((data[3] & 0x0F) << 8);
        return true;
      }
      delay(1);
    }
    return false;
#else
    int16_t x_arr[5], y_arr[5];
    for (uint8_t retry = 0; retry < 3; retry++) {
      uint8_t touched =
          touch.getPoint(x_arr, y_arr, touch.getSupportTouchPoint());
      if (touched == 1) {
        *x = x_arr[0];
        *y = y_arr[0];
        return true;
      }
      delay(1);
    }
    return false;
#endif
  }

  bool readRegister(uint8_t reg, uint8_t *data, size_t length) {
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
      return false;
    }

    size_t read = Wire.requestFrom((uint8_t)CST816_I2C_ADDR, (uint8_t)length);
    if (read != length) {
      return false;
    }

    for (size_t i = 0; i < length; i++) {
      data[i] = Wire.read();
    }
    return true;
  }
};