#pragma once

/*********************
 *      DEFINES
 *********************/
#ifndef BOARD_OEM
#define BOARD_OEM "Wireless-Tag"
#endif
#ifndef BOARD_NAME
#define BOARD_NAME "WT32-SC01-PLUS"
#endif
#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE "LCD"
#endif

/*********************
 *      INCLUDES
 *********************/
#include "displays/wt32.hpp"

/*********************
 *      TYPEDEFS
 *********************/


#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void loop(void) {}
} // namespace board
