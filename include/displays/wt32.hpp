#pragma once

#include "PanelLan.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2

using BoardDisplay = PanelLan;
#define BOARD_DISPLAY_TFT_ARGS (BOARD_SC01_PLUS)
extern BoardDisplay tft;
