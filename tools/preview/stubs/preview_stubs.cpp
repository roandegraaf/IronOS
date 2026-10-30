#include "BSP.h"
#include "Buttons.hpp"
#include "OLED.hpp"
#include "OperatingModeUtilities.h"
#include "TipThermoModel.h"
#include "USBPD.h"
#include "main.hpp"
#include "power.hpp"
#include "preview_state.h"
#include <string.h>

PreviewState previewState;
uint8_t      previewFramebuffer[kFramebufferLen];

TickType_t lastButtonTime;
TickType_t lastMovementTime;

expMovingAverage<uint32_t, wattHistoryFilter> x10WattHistory = {0};
const uint16_t                                powerPWM       = 255;

extern "C" {
TickType_t xTaskGetTickCount(void) { return previewState.tick; }
void       vTaskDelay(TickType_t) {}
void       vTaskDelayUntil(TickType_t *, TickType_t) {}
osStatus   osDelay(uint32_t) { return 0; }

void flash_save_buffer(const uint8_t *, const uint16_t) {}
void flash_read_buffer(uint8_t *buffer, const uint16_t length) { memset(buffer, 0xFF, length); }

int16_t  getHandleTemperature(uint8_t) { return previewState.handleCx10; }
uint16_t getInputVoltageX10(uint16_t, uint8_t) { return previewState.inputVx10; }
bool     getIsPoweredByDCIN() { return previewState.source == PowerSource::DC; }
bool     isTipDisconnected() { return previewState.tipDisconnected; }
uint8_t  getTipResistanceX10() { return previewState.tipRx10; }
}

TemperatureType_t TipThermoModel::getTipInC(bool) { return previewState.tipC; }
TemperatureType_t TipThermoModel::getTipInF(bool) { return (previewState.tipC * 9) / 5 + 32; }
TemperatureType_t TipThermoModel::convertCtoF(TemperatureType_t degC) { return 32 + (degC * 9) / 5; }

bool USBPowerDelivery::fusbPresent() { return previewState.source == PowerSource::PD; }
bool USBPowerDelivery::negotiationComplete() { return previewState.source == PowerSource::PD; }

uint8_t X10WattsToPWM(int32_t, uint8_t) { return previewState.heaterPWM; }
int32_t getX10WattageLimits() { return previewState.x10WattLimit; }

ButtonState getButtonState() { return BUTTON_NONE; }
void        waitForButtonPress() {}
void        waitForButtonPressOrTimeout(TickType_t) {}
void        GUIDelay() {}
bool        checkForUnderVoltage(void) { return false; }

bool I2CBB1::I2C_RegisterWrite(uint8_t, uint8_t, uint8_t) { return true; }
bool I2CBB1::writeRegistersBulk(const uint8_t, const I2C_REG *, const uint8_t) { return true; }
void I2CBB1::Transmit(uint16_t, uint8_t *pData, uint16_t Size) {
  if (Size == FRAMEBUFFER_START + kFramebufferLen) {
    memcpy(previewFramebuffer, pData + FRAMEBUFFER_START, kFramebufferLen);
  }
}
