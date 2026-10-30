#ifndef PREVIEW_STATE_H
#define PREVIEW_STATE_H
#include "FreeRTOS.h"
#include "Types.h"
#include <stdint.h>

enum class PowerSource { DC, QC, PD };

struct PreviewState {
  TickType_t        tick;
  TemperatureType_t tipC;
  int16_t           handleCx10;
  uint16_t          inputVx10;
  PowerSource       source;
  bool              tipDisconnected;
  uint8_t           heaterPWM;
  int32_t           x10WattLimit;
  uint8_t           tipRx10;
};

extern PreviewState previewState;

static const int     kPanelWidth     = 128;
static const int     kPanelHeight    = 32;
static const int     kFramebufferLen = kPanelWidth * (kPanelHeight / 8);
extern uint8_t       previewFramebuffer[kFramebufferLen];
#endif
