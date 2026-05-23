#pragma once


#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 48
#define LCD_SD0 13
#define LCD_SD1 47
#define LCD_SD2 21
#define LCD_SD3 14
#define LCD_RST 11
#define LCD_EN -1 // 17

#define TOUCH_SDA 17
#define TOUCH_SCL 18
#define TOUCH_RST 10
#define TOUCH_IRQ 9

#define BUTTON_HOME 0


#define LV_BUFFER_SIZE (SCREEN_WIDTH * 100)
#define LV_BUFFER_COUNT 1

#define MAX_FILE_OPEN -1

#define USE_DYNAMIC_BUFFERS 0
#define BUFFER_FLAGS -1

#define BOARD_OEM "Viewe"
#define BOARD_NAME "S3 1.5\""
#define DISPLAY_TYPE "AMOLED"
