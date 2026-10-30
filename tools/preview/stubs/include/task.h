#ifndef PREVIEW_TASK_H
#define PREVIEW_TASK_H
#include "FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void *TaskHandle_t;
TickType_t xTaskGetTickCount(void);
void       vTaskDelay(TickType_t ticks);
void       vTaskDelayUntil(TickType_t *previousWakeTime, TickType_t increment);
#ifdef __cplusplus
}
#endif
#endif
