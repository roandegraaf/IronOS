#include "power.hpp"
#include "ui_drawing.hpp"
#include <OperatingModes.h>
#ifdef OLED_128x32
#if defined(OLED_128x32_HERO_UI)
#include "BSP.h"
#include "hero_ui.hpp"

// Positions are right-handed; hero_x() mirrors each element for left-handed mode.
static constexpr int16_t kHeroX         = 2;
static constexpr int16_t kColumnX       = 121;
static constexpr uint8_t kColumnW       = 6;
static constexpr uint8_t kSteps         = 8;
static constexpr uint8_t kStepPitch     = OLED_HEIGHT / kSteps;
static constexpr uint8_t kStepH         = kStepPitch - 1;
static constexpr int16_t kSetpointRight = 117;
static constexpr uint8_t kSetpointY     = 24;
static constexpr uint8_t kHeatLabelY    = 0;

// Rounds up, so any heater output lights at least one step.
static uint8_t litSteps(uint8_t pwm) { return (pwm * kSteps + powerPWM - 1) / powerPWM; }

static void drawHeatColumn() {
  const uint8_t lit   = litSteps(X10WattsToPWM(x10WattHistory.average()));
  const uint8_t left  = hero_x(kColumnX, kColumnW);
  const uint8_t right = left + kColumnW - 1;
  for (uint8_t step = 0; step < kSteps; step++) {
    const uint8_t bottom = OLED_HEIGHT - 1 - step * kStepPitch;
    const uint8_t top    = bottom - kStepH + 1;
    OLED::drawFilledRect(left, top, right, bottom, false);
    if (step >= lit) {
      OLED::drawFilledRect(left + 1, top + 1, right - 1, bottom - 1, true);
    }
  }
}

static void drawSetpoint(bool boostModeOn) {
  const uint16_t setpoint = getSettingValue(boostModeOn ? SettingsOptions::BoostTemp : SettingsOptions::SolderingTemp);
  const uint8_t  digits   = hero_digit_count(setpoint);
  const uint8_t  inkW     = (digits + 1) * TINY_CHAR_W - 1; // TINY cells end in a blank column
  const int16_t  x        = hero_x(kSetpointRight - inkW + 1, inkW);

  ui_draw_hero_small_icon(x, kSetpointY, boostModeOn ? HeroIconBoost : HeroIconArrowRight);
  OLED::setCursor(x + HERO_SMALL_ICON_W, kSetpointY);
  OLED::printNumber(setpoint, digits, FontStyle::TINY);
}

static void drawHeatLabel() {
  const uint8_t inkW = HERO_SMALL_ICON_W - 1;
  ui_draw_hero_small_icon(hero_x(kSetpointRight - inkW + 1, inkW), kHeatLabelY, HeroIconHeat);
}

void ui_draw_soldering_basic_status(bool boostModeOn) {
  ui_draw_hero_live_temperature(kHeroX);
  drawHeatColumn();
  drawHeatLabel();
  drawSetpoint(boostModeOn);
}

#else                       /* !OLED_128x32_HERO_UI */
#ifdef OLED_128x32_DENSE_UI // dense layout + compact font, only on models with the flash for it (see Sequre configuration.h)

// [heat / boost column 16px][LARGE temp + °C 48px][info column: 20.0V / source / set point in the 6x8 font]
// The side columns swap over for left handed mode so the heat indicator stays next to the buttons.
void ui_draw_soldering_basic_status(bool boostModeOn) {
  const bool    leftHanded = OLED::getRotation();
  const int16_t heatX      = leftHanded ? 116 : 0;
  const int16_t tempX      = leftHanded ? 62 : 16;
  const int16_t infoX      = leftHanded ? 8 : 70;

  OLED::setCursor(heatX, 0);
  OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
  if (boostModeOn) {
    OLED::setCursor(heatX, 16);
    OLED::drawSymbol(2); // Up arrow
  }

  OLED::setCursor(tempX, 4); // y=4 centres the 24px digits
  ui_draw_tip_temperature(true, FontStyle::LARGE);

  OLED::setCursor(infoX, 0); // info rows on strip boundaries: unaligned y would clobber the neighbouring row
  {
    uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
    OLED::print(SmallSymbolDot, FontStyle::TINY);
    OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
    OLED::print(SmallSymbolVolts, FontStyle::TINY);
  }
  OLED::setCursor(infoX, 8);
#ifdef POW_DC
  if (getIsPoweredByDCIN() && getSettingValue(SettingsOptions::MinDCVoltageCells)) {
    ui_draw_power_source_icon(); // battery gauge
  } else
#endif
  {
    OLED::print(PowerSourceNames[getPowerSourceNumber()], FontStyle::TINY, 2);
  }
  OLED::setCursor(infoX, 16);
  OLED::print(SmallSymbolSpace, FontStyle::TINY);
  OLED::printNumber(getSettingValue(boostModeOn ? SettingsOptions::BoostTemp : SettingsOptions::SolderingTemp), 3, FontStyle::TINY);
  OLED::print(SmallSymbolDegreeSign, FontStyle::TINY);
#ifdef DUAL_TIP_SOCKETS
  OLED::setCursor(infoX + 6, 24);
  OLED::print(getActiveTipChannel() ? SmallSymbolSocketTS : SmallSymbolSocketT12, FontStyle::TINY);
#endif
}

#else  /* upstream layout */

void ui_draw_soldering_basic_status(bool boostModeOn) {
  OLED::setCursor(0, 0);
  // We switch the layout direction depending on the orientation of the oled
  if (OLED::getRotation()) {
    // battery
    ui_draw_power_source_icon();
    // Space out gap between battery <-> temp
    OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    // Draw current tip temp (y=4 centres the 24px number; +12 nudges it one digit right; restore after)
    OLED::setCursor(OLED::getCursorX() + 12, 4);
    ui_draw_tip_temperature(true, FontStyle::LARGE);
    OLED::setCursor(OLED::getCursorX(), 0);

    // We draw boost arrow if boosting,
    // or else gap temp <-> heat indicator
    if (boostModeOn) {
      OLED::drawSymbol(2);
    } else {
      OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    }

    // Draw heating/cooling symbols
    OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
  } else {
    // Draw heating/cooling symbols
    OLED::drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()));
    // We draw boost arrow if boosting,
    // or else gap temp <-> heat indicator
    if (boostModeOn) {
      OLED::drawSymbol(2);
    } else {
      OLED::print(LargeSymbolSpace, FontStyle::LARGE);
    }
    // Draw current tip temp (y=4 centres the 24px number; +12 nudges it one digit right; restore after)
    OLED::setCursor(OLED::getCursorX() + 12, 4);
    ui_draw_tip_temperature(true, FontStyle::LARGE);
    OLED::setCursor(OLED::getCursorX(), 0);
    // Power source icon near the right edge (matches the simplified idle screen)
    OLED::setCursor(116, 0);
    ui_draw_power_source_icon();
  }
}
#endif /* OLED_128x32_DENSE_UI */
#endif /* OLED_128x32_HERO_UI */
#endif
