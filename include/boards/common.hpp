#pragma once

#ifndef BOARD_SW_ROTATION
#define BOARD_SW_ROTATION 0
#endif

#ifndef BOARD_ROTATION_LOCKED
#define BOARD_ROTATION_LOCKED 0
#endif

#ifndef BOARD_ROTATION_VALUE
#define BOARD_ROTATION_VALUE -1
#endif

#ifndef BOARD_ROTATION_OFFSET
#define BOARD_ROTATION_OFFSET 0
#endif

#ifndef BOARD_HAS_TOUCH
#define BOARD_HAS_TOUCH 0
#endif

#ifndef BOARD_HAS_CUSTOM_BRIGHTNESS
#define BOARD_HAS_CUSTOM_BRIGHTNESS 0
#endif

#ifndef BOARD_HAS_WAKE_INPUT
#define BOARD_HAS_WAKE_INPUT BOARD_HAS_TOUCH
#endif

#ifndef BOARD_ENABLE_SCREEN_TIMEOUT
#define BOARD_ENABLE_SCREEN_TIMEOUT BOARD_HAS_WAKE_INPUT
#endif

#ifndef BOARD_USE_ROUNDER_CB
#define BOARD_USE_ROUNDER_CB 0
#endif


#ifndef BOARD_OEM
#define BOARD_OEM "Unknown"
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Unknown"
#endif

#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "Unknown"
#endif

#ifndef UI_MODE
#define UI_MODE 0
#endif
