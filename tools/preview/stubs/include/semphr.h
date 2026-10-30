#ifndef PREVIEW_SEMPHR_H
#define PREVIEW_SEMPHR_H
#include "FreeRTOS.h"
typedef void *SemaphoreHandle_t;
typedef struct {
  int unused;
} StaticSemaphore_t;
#endif
