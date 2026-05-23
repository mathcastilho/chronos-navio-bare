#pragma once

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 280
#define OFFSET_X 0
#define OFFSET_Y 20
#define RGB_ORDER true

// touch
#define I2C_SDA 11
#define I2C_SCL 10
#define TP_INT 14
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 4
#define CS 5
#define RST 8

#define BL 15

#define VIBRATION_PIN -1

#define BUZZER_PIN -1 //33

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2

#define LCD_CS -1
#define LCD_SCK -1
#define LCD_SD0 -1
#define LCD_SD1 -1
#define LCD_SD2 -1
#define LCD_SD3 -1
#define LCD_RST -1
#define LCD_EN -1

#define TOUCH_SDA -1
#define TOUCH_SCL -1
#define TOUCH_RST -1
#define TOUCH_IRQ -1

#define ENCODER_A -1
#define ENCODER_B -1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1
