#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32
#if defined(OLED_128x32_HERO_UI)
#include "TipThermoModel.h"
#include "hero_ui.hpp"

// Positions are right-handed; hero_x() mirrors the hero, the divider and the row block as a whole for left-handed mode.
static constexpr int16_t kHeroX       = 2;
static constexpr int16_t kDividerX    = 72;
static constexpr int16_t kRowsX       = 76;
static constexpr uint8_t kRowsW       = OLED_WIDTH - kRowsX;
static constexpr uint8_t kSourceChars = 2;
// Rows keep to disjoint 8 px strips (0, 1-2, 3): an unaligned TINY glyph overwrites whole column bytes of both strips it touches.
static constexpr uint8_t kSetRowY    = 0;
static constexpr uint8_t kSourceRowY = 12;
static constexpr uint8_t kHandleRowY = 24;

static void drawTemperatureRow(int16_t blockX, uint8_t y, const char *label, uint16_t value) {
  const uint8_t digits = hero_digit_count(value);
  OLED::setCursor(blockX, y);
  OLED::print(label, FontStyle::TINY);
  OLED::setCursor(blockX + kRowsW - (digits + 1) * TINY_CHAR_W, y);
  OLED::printNumber(value, digits, FontStyle::TINY);
  OLED::print(SmallSymbolDegreeSign, FontStyle::TINY);
}

static void drawSourceRow(int16_t blockX) {
  const uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  OLED::setCursor(blockX, kSourceRowY);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, kSourceChars);
  ui_draw_tiny_voltage(blockX + kRowsW - tiny_voltage_width(voltX10), kSourceRowY, voltX10);
}

static uint16_t handleTemperature() {
  const int16_t           handleX10 = getHandleTemperature(0);
  const TemperatureType_t handleC   = handleX10 > 0 ? handleX10 / 10 : 0;
  return getSettingValue(SettingsOptions::TemperatureInF) ? TipThermoModel::convertCtoF(handleC) : handleC;
}

void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  const int16_t blockX   = hero_x(kRowsX, kRowsW);
  const int16_t dividerX = hero_x(kDividerX, 1);

  ui_draw_hero_tip_temperature(kHeroX, tipTemp);
  drawTemperatureRow(blockX, kSetRowY, SmallSymbolSetPoint, getSettingValue(SettingsOptions::SolderingTemp));
  drawSourceRow(blockX);
  drawTemperatureRow(blockX, kHandleRowY, SmallSymbolHandle, handleTemperature());
  OLED::drawFilledRect(dividerX, 0, dividerX, OLED_HEIGHT - 1, false);
}

#else                       /* !OLED_128x32_HERO_UI */
#ifdef OLED_128x32_DENSE_UI // dense layout + compact font, only on models with the flash for it (see Sequre configuration.h)

extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

/*
 * Idle data view (right handed; blocks swap for left handed):
 *   x 0..59                    x 60..127 (6x8 TINY font)
 *   | 123°C (8x16)            | 41° 5.5Ω   handle / cartridge |  y 0
 *   |                         | Max130W    power limit        |  y 8
 *   |  320°  set point        | 11kHz      chop frequency     |  y 16
 *   | PD 20.1V                |                               |  y 24
 */
void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t mainX      = leftHanded ? 68 : 0;
  const int16_t infoX      = leftHanded ? 0 : 60;

  if (isTipDisconnected()) {
    if (leftHanded) {
      OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
      OLED::setCursor(-1, 4);
    } else {
      OLED::drawArea(0, 0, 56, 32, disconnectedTip);
      OLED::setCursor(56, 4);
    }
    uint32_t Vlt = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(Vlt / 10, 2, FontStyle::LARGE);
    OLED::print(LargeSymbolDot, FontStyle::LARGE);
    OLED::printNumber(Vlt % 10, 1, FontStyle::LARGE);
    OLED::setCursor(OLED::getCursorX(), 12);
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
    return;
  }

  if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (tipTemp > 55) && (xTaskGetTickCount() % 1000 < 300))) {
    // Blink temp if setting enable and temp < 55°
    OLED::setCursor(mainX, 0);
    ui_draw_tip_temperature(true, FontStyle::SMALL);
  }
  OLED::setCursor(mainX, 16);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::TINY);
  OLED::print(SmallSymbolDegreeSign, FontStyle::TINY);
  OLED::setCursor(mainX, 24);
  OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, 2);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  {
    uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolVolts, FontStyle::TINY);
  }

  // Info block
  OLED::setCursor(infoX, 0);
  OLED::printNumber(getHandleTemperature(0) / 10, 2, FontStyle::TINY);
  OLED::print(SmallSymbolDegreeSign, FontStyle::TINY);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  {
    uint8_t tipRx10 = getTipResistanceX10();
    OLED::printNumber(tipRx10 / 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(tipRx10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolOhm, FontStyle::TINY);
  }
  OLED::setCursor(infoX, 8);
  {
    int32_t x10Limit = getX10WattageLimits();
    OLED::print(SmallSymbolMax, FontStyle::TINY);
    OLED::printNumber(x10Limit > 0 ? (uint16_t)(x10Limit / 10) : 0, 3, FontStyle::TINY);
    OLED::print(SmallSymbolWatts, FontStyle::TINY);
  }
#ifdef TIP_CURRENT_LIMIT_CHOP
  OLED::setCursor(infoX, 16);
  OLED::printNumber((getTipChopFrequencyHzX10() + 5000) / 10000, 2, FontStyle::TINY);
  OLED::print(SmallSymbolKiloHertz, FontStyle::TINY);
#endif
#ifdef DUAL_TIP_SOCKETS
  OLED::setCursor(infoX, 24);
  OLED::print(getActiveTipChannel() ? SmallSymbolSocketTS : SmallSymbolSocketT12, FontStyle::TINY);
#endif
}

#else  /* upstream layout */

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

void ui_draw_homescreen_detailed(TemperatureType_t tipTemp) {
  if (isTipDisconnected()) {
    if (OLED::getRotation()) {
      // in right handed mode we want to draw over the first part
      OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
    } else {
      OLED::drawArea(0, 0, 56, 32, disconnectedTip);
    }
    if (OLED::getRotation()) {
      OLED::setCursor(-1, 0);
    } else {
      OLED::setCursor(56, 0);
    }
    uint32_t Vlt = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(Vlt / 10, 2, FontStyle::LARGE);
    OLED::print(LargeSymbolDot, FontStyle::LARGE);
    OLED::printNumber(Vlt % 10, 1, FontStyle::LARGE);
    if (OLED::getRotation()) {
      OLED::setCursor(48, 8);
    } else {
      OLED::setCursor(91, 8);
    }
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
  } else {
    // One-line LARGE (12x24) tip temperature flush to one edge, vertically
    // centred; two SMALL (8x16) status rows (set-temp, voltage) flush to the
    // other edge. Sides flip with rotation.
    const bool    rot      = OLED::getRotation();
    const uint8_t statusW  = 5 * 8;              // "NNN°C" / "NN.NV" are 5 cells in the 8x16 small font
    const uint8_t tempW    = (3 * 12) + (2 * 8); // 3 large digits + small "°C" = 52px
    const uint8_t tempZone = 68;                 // temperature right-aligned within this zone (left of the status block)
    const int16_t tempX    = rot ? (OLED_WIDTH - tempZone) : (tempZone - tempW);
    const int16_t statusX  = rot ? 0 : (OLED_WIDTH - statusW);

    if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (tipTemp > 55) && (xTaskGetTickCount() % 1000 < 300))) {
      // Blink temp if setting enable and temp < 55° (OFF 300ms / ON 700ms)
      OLED::setCursor(tempX, 4); // 24px number, vertically centred (4px above/below)
      ui_draw_tip_temperature(false, FontStyle::LARGE);
      // Degree + unit in the small font, bottom-aligned with the 24px number (like the TS100)
      OLED::setCursor(OLED::getCursorX(), 12);
      OLED::printSymbolDeg(FontStyle::SMALL);
    }

    // Set temperature (top row)
    OLED::setCursor(statusX, 0);
    OLED::printNumber(getSettingValue(SettingsOptions::SolderingTemp), 3, FontStyle::SMALL);
    OLED::printSymbolDeg(FontStyle::SMALL);

    // Input voltage (bottom row)
    OLED::setCursor(statusX, 16);
    printVoltage(); // draw voltage then symbol (v)
    OLED::print(SmallSymbolVolts, FontStyle::SMALL);
  }
}
#endif /* OLED_128x32_DENSE_UI */
#endif /* OLED_128x32_HERO_UI */
#endif
