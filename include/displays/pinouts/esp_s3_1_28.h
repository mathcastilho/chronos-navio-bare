#pragma once

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 6
#define I2C_SCL 7
#define TP_INT 5
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 10
#define MOSI 11
#define MISO 12
#define DC 8
#define CS 9
#define RST 14

#define BL 2

#define VIBRATION_PIN -1

#define BUZZER_PIN -1

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

#define BOARD_OEM "Waveshare"
#define BOARD_NAME "S3 1.28\""
#define DISPLAY_TYPE "LCD"
