/*
   MIT License

  Copyright (c) 2026 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#pragma once

#if defined(ELECROW_C3)
#include "pinouts/elecrow_c3_1_28.h"
#elif defined(ESPC3)
#include "pinouts/esp_c3_1_28.h"
#elif defined(ESPS3_1_28)
#include "pinouts/esp_s3_1_28.h"
#elif defined(ESPS3_1_69)
#include "pinouts/esp_s3_1_69.h"
#elif defined(M5_STACK_DIAL)
#include "pinouts/m5_stack_dial.h"
#elif defined(VIEWE_SMARTRING)
#include "pinouts/viewe_smartring.h"
#elif defined(VIEWE_KNOB_15)
#include "pinouts/viewe_knob_1_5.h"
#elif defined(VIEWE_S3_1_5)
#include "pinouts/viewe_s3_1_5.h"
#elif defined(ESPS3_2_06)
#include "pinouts/esp_s3_2_06.h"
#else
#include "pinouts/default_esp32.h"
#endif
