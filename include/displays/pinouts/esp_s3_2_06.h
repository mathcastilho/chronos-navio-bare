#pragma once

#define SCREEN_WIDTH 410
#define SCREEN_HEIGHT 502
#define OFFSET_X -1
#define OFFSET_Y -1
#define RGB_ORDER false

#define I2C_SDA -1
#define I2C_SCL -1
#define TP_INT -1
#define TP_RST -1

#define SPI -1

#define SCLK -1
#define MOSI -1
#define MISO -1
#define DC -1
#define CS -1
#define RST -1
#define BL -1

#define VIBRATION_PIN -1

#define BUZZER_PIN -1

#define LCD_CS 12
#define LCD_SCK 11
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 8
#define LCD_EN -1

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 9
#define TOUCH_IRQ 38

#define ENCODER_A -1
#define ENCODER_B -1

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 200)
#define LV_BUFFER_COUNT 1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 1

#define BUFFER_FLAGS MALLOC_CAP_SPIRAM

#define BOARD_OEM "Waveshare"
#define BOARD_NAME "S3 2.06\""
#define DISPLAY_TYPE "AMOLED"
