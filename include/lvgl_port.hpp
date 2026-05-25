#pragma once


/*********************
 *      INCLUDES
 *********************/

#include <lvgl.h>
#include <stdint.h>

/*********************
 *      TYPEDEFS
 *********************/

typedef bool (*LvglPortScreenAwakeFn)(void);
typedef void (*LvglPortScreenActivityFn)(uint32_t extra_ms);


/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Register optional screen wake/activity callbacks used by the touch driver.
 *
 * The LVGL input callback uses these hooks to wake the screen without passing
 * the first wake tap through to the UI. Pass nullptr for either callback when a
 * board does not need screen wake gating.
 *
 * @param is_awake Returns true when the screen is already awake.
 * @param activity Notifies the app that input activity happened.
 */
void lvgl_port_set_screen_callbacks(LvglPortScreenAwakeFn is_awake,
                                    LvglPortScreenActivityFn activity);

/**
 * Initialize LVGL for the selected board profile.
 *
 * Creates the display, assigns draw buffers, registers the flush callback, and
 * conditionally registers touch and invalid-area rounding callbacks based on the
 * active board profile macros.
 */
void lvgl_port_init(void);

/**
 * Convert a board rotation value into an LVGL display rotation enum.
 *
 * Rotation values outside LVGL's 0-3 range fall back to
 * LV_DISPLAY_ROTATION_0.
 *
 * @param rotation Board rotation value, usually 0-3.
 * @return Matching LVGL display rotation.
 */
lv_display_rotation_t lvgl_port_get_rotation(uint8_t rotation);

