#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#if LV_USE_OS == LV_OS_CUSTOM

typedef struct {
    TaskHandle_t handle;
} lv_thread_t;

typedef struct {
    SemaphoreHandle_t handle;
} lv_mutex_t;

typedef struct {
    SemaphoreHandle_t handle;
} lv_thread_sync_t;

#endif

#ifdef __cplusplus
}
#endif
