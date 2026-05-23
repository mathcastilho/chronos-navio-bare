#include "src/osal/lv_os_private.h"

#if LV_USE_OS == LV_OS_CUSTOM

static UBaseType_t lv_prio_to_freertos(lv_thread_prio_t prio) {
  UBaseType_t max_prio = configMAX_PRIORITIES - 1;
  UBaseType_t mapped_prio = tskIDLE_PRIORITY + 1;

  switch (prio) {
  case LV_THREAD_PRIO_LOWEST:
    mapped_prio = tskIDLE_PRIORITY + 1;
    break;
  case LV_THREAD_PRIO_LOW:
    mapped_prio = tskIDLE_PRIORITY + 2;
    break;
  case LV_THREAD_PRIO_MID:
    mapped_prio = tskIDLE_PRIORITY + 3;
    break;
  case LV_THREAD_PRIO_HIGH:
    mapped_prio = tskIDLE_PRIORITY + 4;
    break;
  case LV_THREAD_PRIO_HIGHEST:
    mapped_prio = tskIDLE_PRIORITY + 5;
    break;
  }

  return mapped_prio > max_prio ? max_prio : mapped_prio;
}

extern "C" lv_result_t lv_thread_init(lv_thread_t *thread,
                                      const char *const name,
                                      lv_thread_prio_t prio,
                                      void (*callback)(void *),
                                      size_t stack_size, void *user_data) {
  if (thread == NULL || callback == NULL) {
    return LV_RESULT_INVALID;
  }

  BaseType_t ret = xTaskCreate(callback, name, stack_size,
                               user_data, lv_prio_to_freertos(prio),
                               &thread->handle);

  return ret == pdPASS ? LV_RESULT_OK : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_thread_delete(lv_thread_t *thread) {
  if (thread == NULL || thread->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  vTaskDelete(thread->handle);
  thread->handle = NULL;
  return LV_RESULT_OK;
}

extern "C" lv_result_t lv_mutex_init(lv_mutex_t *mutex) {
  if (mutex == NULL) {
    return LV_RESULT_INVALID;
  }

  mutex->handle = xSemaphoreCreateRecursiveMutex();
  return mutex->handle != NULL ? LV_RESULT_OK : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_mutex_lock(lv_mutex_t *mutex) {
  if (mutex == NULL || mutex->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  return xSemaphoreTakeRecursive(mutex->handle, portMAX_DELAY) == pdTRUE
             ? LV_RESULT_OK
             : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_mutex_lock_isr(lv_mutex_t *mutex) {
  LV_UNUSED(mutex);
  return LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_mutex_unlock(lv_mutex_t *mutex) {
  if (mutex == NULL || mutex->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  return xSemaphoreGiveRecursive(mutex->handle) == pdTRUE ? LV_RESULT_OK
                                                          : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_mutex_delete(lv_mutex_t *mutex) {
  if (mutex == NULL || mutex->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  vSemaphoreDelete(mutex->handle);
  mutex->handle = NULL;
  return LV_RESULT_OK;
}

extern "C" lv_result_t lv_thread_sync_init(lv_thread_sync_t *sync) {
  if (sync == NULL) {
    return LV_RESULT_INVALID;
  }

  sync->handle = xSemaphoreCreateBinary();
  return sync->handle != NULL ? LV_RESULT_OK : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_thread_sync_wait(lv_thread_sync_t *sync) {
  if (sync == NULL || sync->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  return xSemaphoreTake(sync->handle, portMAX_DELAY) == pdTRUE
             ? LV_RESULT_OK
             : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_thread_sync_signal(lv_thread_sync_t *sync) {
  if (sync == NULL || sync->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  return xSemaphoreGive(sync->handle) == pdTRUE ? LV_RESULT_OK
                                                : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_thread_sync_signal_isr(lv_thread_sync_t *sync) {
  if (sync == NULL || sync->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  BaseType_t higher_priority_task_woken = pdFALSE;
  BaseType_t ret =
      xSemaphoreGiveFromISR(sync->handle, &higher_priority_task_woken);

  if (higher_priority_task_woken == pdTRUE) {
    portYIELD_FROM_ISR();
  }

  return ret == pdTRUE ? LV_RESULT_OK : LV_RESULT_INVALID;
}

extern "C" lv_result_t lv_thread_sync_delete(lv_thread_sync_t *sync) {
  if (sync == NULL || sync->handle == NULL) {
    return LV_RESULT_INVALID;
  }

  vSemaphoreDelete(sync->handle);
  sync->handle = NULL;
  return LV_RESULT_OK;
}

extern "C" void lv_sleep_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

#endif
