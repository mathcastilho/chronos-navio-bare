# Display, Touch, and Board Configuration

This project selects hardware at compile time from the PlatformIO environment
by defining `BOARD_PROFILE_INCLUDE`, for example
`-D BOARD_PROFILE_INCLUDE=\"boards/viewe-smartring.hpp\"`.

## Selection Flow

1. `platformio.ini` defines `BOARD_PROFILE_INCLUDE` per environment.
2. `include/board_profile.hpp` includes that profile, or `boards/default.hpp`
   when no profile is provided.
3. The board profile defines the board constants, chooses the display panel,
   chooses the touch driver, and exposes a global `tft` object.
4. `src/main.cpp` only talks to `tft` and `board::*` hooks.

## Layers

Board constants live at the top of each `include/boards/*.hpp` file under the
`DEFINES` section.

They include GPIOs, resolution, buffer size, board name, display type, memory
settings, and vendor init tables that are pure data. Runtime behavior belongs
in panel, touch, or board hook sections.

Panel headers live in `include/displays/panels/`.

They own pixel output and backlight behavior. Examples:

- `gc9a01_spi.hpp`
- `ili9488_parallel16.hpp`
- `co5300_qspi.hpp`
- `st77916_qspi.hpp`
- `m5_dial.hpp`

Touch headers live in `include/displays/touch/`.

They own touch-controller setup and reading. Examples:

- `lovyan_cst816s.hpp`
- `lovyan_ft5x06.hpp`
- `cstxxx.hpp`
- `ft6x36.hpp`
- `cst816.hpp`
- `none.hpp`
- `lovyan_none.hpp`

Board profiles live in `include/boards/`.

They compose the hardware pieces and provide board-specific hooks:

```cpp
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void loop(void) {}
}
```

Use those hooks for board-only logic such as power rails, IO expanders,
encoders, RFID, sensors, or periodic board updates.

Board profiles also expose small capability flags used by the LVGL port:

- `BOARD_HAS_TOUCH`: set to `0` when no touch device should be registered.
- `BOARD_HAS_WAKE_INPUT`: set to `1` when touch or a button can wake the screen.
- `BOARD_ENABLE_SCREEN_TIMEOUT`: enables automatic screen timeout.
- `BOARD_USE_ROUNDER_CB`: set to `1` for panels, such as AMOLED, that require
  invalidated LVGL areas to be aligned to even coordinates.
- `BOARD_ROTATION_OFFSET`: maps logical UI rotation to panel rotation.

LovyanGFX PWM backlights use `TFT_BL_INVERTED`, which defaults to `false`.
Boards with active-low backlights define it as `true` before including the
panel.

## Adding a Board

Create a board profile:

```cpp
#pragma once

/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define LV_BUFFER_SIZE (SCREEN_WIDTH * 40)
#define LV_BUFFER_COUNT 2

/*********************
 *      INCLUDES
 *********************/
#include "displays/display_wrapper.hpp"
#include "displays/panels/gc9a01_spi.hpp"
#include "displays/touch/lovyan_cst816s.hpp"

/*********************
 *      TYPEDEFS
 *********************/
using BoardDisplay =
    display::DisplayWrapper<GC9A01SpiPanel, LovyanCST816STouch>;
extern BoardDisplay tft;

#include "boards/common.hpp"

/*********************
 *      BOARD HOOKS
 *********************/
namespace board {
inline void before_display_init(void) {}
inline void after_display_init(void) {}
inline void after_ui_init(void) {}
inline void set_brightness(uint8_t value) {}
inline bool wakeup_activity(void) { return false; }
inline bool screen_toggle_requested(void) { return false; }
inline void loop(void) {}
}
```

Add a PlatformIO environment with a matching `BOARD_PROFILE_INCLUDE` flag.
`src/board_profile.cpp` owns the single global `tft` instance.

## Display and Touch Composition

Use `display::DisplayWrapper<Panel, Touch>` for both Arduino_GFX and LovyanGFX
boards:

```cpp
using BoardDisplay = display::DisplayWrapper<CO5300QspiPanel, CSTXXXTouch>;
```

LovyanGFX panels use the same wrapper. Their touch drivers attach themselves to
the panel during construction:

```cpp
using BoardDisplay = display::DisplayWrapper<GC9A01SpiPanel, LovyanCST816STouch>;
```

Use a direct adapter when the board library owns the display and touch, as with
M5Dial:

```cpp
using BoardDisplay = M5DialDisplay;
extern BoardDisplay tft;
```

For boards without touch, use `NoTouch` or `LovyanNoTouch`, depending on the
display backend.

## Pinouts in Board Files

Keep board constants in the board profile even when the list is long. If a board
needs a large vendor init table, place it in the `DEFINES` section as pure data,
as `viewe-echo-ear.hpp` does.
