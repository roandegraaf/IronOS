#ifndef PREVIEW_CMSIS_OS_H
#define PREVIEW_CMSIS_OS_H
#include "FreeRTOS.h"
#include "task.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef int          osStatus;
typedef TaskHandle_t osThreadId;
osStatus osDelay(uint32_t millisec);
#ifdef __cplusplus
}
#endif
#endif
