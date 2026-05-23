#pragma once

#if defined(ELECROW_C3)
#include "boards/elecrow_c3_1_28.hpp"
#elif defined(ESPC3)
#include "boards/esp_c3_1_28.hpp"
#elif defined(ESPS3_1_28)
#include "boards/esp_s3_1_28.hpp"
#elif defined(ESPS3_1_69)
#include "boards/esp_s3_1_69.hpp"
#elif defined(M5_STACK_DIAL)
#include "boards/m5_stack_dial.hpp"
#elif defined(VIEWE_SMARTRING)
#include "boards/viewe_smartring.hpp"
#elif defined(VIEWE_KNOB_15)
#include "boards/viewe_knob_1_5.hpp"
#elif defined(VIEWE_S3_1_5)
#include "boards/viewe_s3_1_5.hpp"
#elif defined(ESPS3_2_06)
#include "boards/esp_s3_2_06.hpp"
#elif defined(ELECROW_35)
#include "boards/elecrow_3_5.hpp"
#elif defined(WT32_SC01_PLUS)
#include "boards/wt32_sc01_plus.hpp"
#elif defined(ECHO_EAR)
#include "boards/echo_ear.hpp"
#else
#include "boards/default.hpp"
#endif
