#include "ui_drawing.hpp"
#ifdef OLED_128x32
#if defined(OLED_128x32_HERO_UI)
#include "hero_ui.hpp"

// Positions are right-handed; hero_x() mirrors each element for left-handed mode.
static constexpr int16_t kHeroX     = 2;
static constexpr int16_t kColumnX   = 88;
static constexpr uint8_t kColumnW   = OLED_WIDTH - kColumnX;
static constexpr uint8_t kVoltsY    = 4;
static constexpr uint8_t kBoxTop    = 18;
static constexpr uint8_t kBoxBottom = 30;

static constexpr uint8_t kBatteryFillLeft   = 2;
static constexpr uint8_t kBatteryFillRight  = 4;
static constexpr uint8_t kBatteryFillBottom = 10;
static constexpr uint8_t kBatteryFillRows   = 6;
static constexpr uint8_t kBatteryLevels     = 10;

// Same 1-10 scale as ui_draw_power_source_icon: per-cell voltage above the cut-off, 0.1 V per level.
static uint8_t batteryLevel(uint32_t voltX10) {
  const uint8_t  cells      = getSettingValue(SettingsOptions::MinDCVoltageCells) + 2;
  const uint32_t minCellX10 = getSettingValue(SettingsOptions::MinVoltageCells);
  const uint32_t cellX10    = voltX10 / cells;
  const uint32_t above      = cellX10 > minCellX10 ? cellX10 - minCellX10 : 0;
  return (above < kBatteryLevels - 1 ? above : kBatteryLevels - 1) + 1;
}

static void drawBattery(int16_t x, uint32_t voltX10) {
  const uint8_t rows = (batteryLevel(voltX10) * kBatteryFillRows + kBatteryLevels - 1) / kBatteryLevels;
  ui_draw_hero_icon(x, HeroIconBatteryOutline);
  OLED::drawFilledRect(x + kBatteryFillLeft, kBatteryFillBottom - rows + 1, x + kBatteryFillRight, kBatteryFillBottom, false);
}

static void drawPowerSourceIcon(int16_t x, uint32_t voltX10) {
  if (!getIsPoweredByDCIN()) {
    ui_draw_hero_icon(x, HeroIconUsbC);
  } else if (getSettingValue(SettingsOptions::MinDCVoltageCells)) {
    drawBattery(x, voltX10);
  } else {
    ui_draw_hero_icon(x, HeroIconBolt);
  }
}

static void drawInputVoltage() {
  const uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  const uint8_t  groupW  = HERO_ICON_W + 1 + tiny_voltage_width(voltX10);
  const int16_t  groupX  = hero_x(OLED_WIDTH - groupW, groupW);

  drawPowerSourceIcon(groupX, voltX10);
  ui_draw_tiny_voltage(groupX + HERO_ICON_W + 1, kVoltsY, voltX10);
}

static void drawSetpointBox() {
  const uint16_t setpoint = getSettingValue(SettingsOptions::SolderingTemp);
  const uint8_t  digits   = hero_digit_count(setpoint);
  const uint8_t  inkW     = (digits + 1) * TINY_CHAR_W - 1; // TINY cells end in a blank column
  const int16_t  boxX     = hero_x(kColumnX, kColumnW);
  const uint8_t  boxRight = boxX + kColumnW - 1;

  // Text first: unaligned TINY glyphs overwrite whole column bytes, the inversion then fills around them.
  OLED::setCursor(boxX + (kColumnW - inkW + 1) / 2, kBoxTop + 3);
  OLED::printNumber(setpoint, digits, FontStyle::TINY);
  OLED::print(SmallSymbolDegreeSign, FontStyle::TINY);
  OLED::invertRect(boxX, kBoxTop, boxRight, kBoxBottom);
  OLED::drawFilledRect(boxX, kBoxTop, boxX, kBoxTop, true);
  OLED::drawFilledRect(boxRight, kBoxTop, boxRight, kBoxTop, true);
  OLED::drawFilledRect(boxX, kBoxBottom, boxX, kBoxBottom, true);
  OLED::drawFilledRect(boxRight, kBoxBottom, boxRight, kBoxBottom, true);
}

void ui_draw_homescreen_simplified(TemperatureType_t tipTemp) {
  ui_draw_hero_tip_temperature(kHeroX, tipTemp);
  drawInputVoltage();
  drawSetpointBox();
}

#else                       /* !OLED_128x32_HERO_UI */
#ifdef OLED_128x32_DENSE_UI // dense layout + compact font, only on models with the flash for it (see Sequre configuration.h)

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

/*
 * [button A 0..52][button B 50..102][ 20.0 / V  104..127 ]
 * The round pictograms only use columns 0..52 of their 56 px bitmaps and their outer columns are
 * blank except for the little pointer nub, so button B can sit 8 px further left without touching A.
 * That leaves 24 px for the input voltage in the 6x8 font instead of two stacked digits.
 */
static void drawPowerSourceSmall(int16_t x) {
#ifdef POW_DC
  if (getIsPoweredByDCIN() && getSettingValue(SettingsOptions::MinDCVoltageCells)) {
    // Lithium pack: keep the battery gauge
    OLED::setCursor(x + 6, 8);
    ui_draw_power_source_icon();
    return;
  }
#endif
  uint32_t voltX10 = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  OLED::setCursor(x, 8);
  OLED::printNumber(voltX10 / 10, 2, FontStyle::TINY);
  OLED::print(SmallSymbolDot, FontStyle::TINY);
  OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
  OLED::setCursor(x + 9, 16);
  OLED::print(SmallSymbolVolts, FontStyle::TINY);
#ifdef DUAL_TIP_SOCKETS
  OLED::setCursor(x + 3, 24);
  OLED::print(getActiveTipChannel() ? SmallSymbolSocketTS : SmallSymbolSocketT12, FontStyle::TINY);
#endif
}

void ui_draw_homescreen_simplified(TemperatureType_t tipTemp) {
  const bool    leftHanded             = OLED::getRotation();
  const int16_t buttonAX               = leftHanded ? 71 : 0;
  const int16_t buttonBX               = leftHanded ? 21 : 50;
  const int16_t sourceX                = leftHanded ? 0 : 104;
  bool          tempOnDisplay          = false;
  bool          tipDisconnectedDisplay = false;

  OLED::drawArea(buttonAX, 0, 56, 32, leftHanded ? buttonAF : buttonA);
  OLED::drawArea(buttonBX, 0, 56, 32, leftHanded ? buttonBF : buttonB);
  drawPowerSourceSmall(sourceX);

  if (tipTemp > 55) {
    tempOnDisplay = true;
  } else if (tipTemp < 45) {
    tempOnDisplay = false;
  }
  if (isTipDisconnected()) {
    tempOnDisplay          = false;
    tipDisconnectedDisplay = true;
  }
  if (tempOnDisplay || tipDisconnectedDisplay) {
    // Draw the temp (or the missing tip symbol) over the start soldering button.
    // Clear the whole A pictogram (left handed: it is drawn 3 px further right inside its bitmap)
    OLED::fillArea(leftHanded ? 74 : 0, 0, leftHanded ? 54 : 50, 32, 0);
    if (!tipDisconnectedDisplay) {
      if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300))) {
        OLED::setCursor(leftHanded ? 78 : 2, 4); // y=4 centres the 24px digits
        ui_draw_tip_temperature(true, FontStyle::LARGE);
      }
    } else {
      OLED::drawArea(buttonAX, 0, 56, 32, leftHanded ? disconnectedTipF : disconnectedTip);
    }
  }
}

#else  /* upstream layout */

extern uint8_t buttonAF[sizeof(buttonA)];
extern uint8_t buttonBF[sizeof(buttonB)];
extern uint8_t disconnectedTipF[sizeof(disconnectedTip)];

void ui_draw_homescreen_simplified(TemperatureType_t tipTemp) {
  bool tempOnDisplay          = false;
  bool tipDisconnectedDisplay = false;
  if (OLED::getRotation()) {
    OLED::drawArea(68, 0, 56, 32, buttonAF);
    OLED::drawArea(12, 0, 56, 32, buttonBF);
    OLED::setCursor(0, 0);
    ui_draw_power_source_icon();
  } else {
    OLED::drawArea(0, 0, 56, 32, buttonA);  // Needs to be flipped so button ends up
    OLED::drawArea(58, 0, 56, 32, buttonB); // on right side of screen
    OLED::setCursor(116, 0);
    ui_draw_power_source_icon();
  }
  tipDisconnectedDisplay = false;
  if (tipTemp > 55) {
    tempOnDisplay = true;
  } else if (tipTemp < 45) {
    tempOnDisplay = false;
  }
  if (isTipDisconnected()) {
    tempOnDisplay          = false;
    tipDisconnectedDisplay = true;
  }
  if (tempOnDisplay || tipDisconnectedDisplay) {
    // draw temp over the start soldering button
    // Location changes on screen rotation
    if (OLED::getRotation()) {
      // in right handed mode we want to draw over the first part
      OLED::fillArea(68, 0, 56, 32, 0); // clear the area for the temp
      OLED::setCursor(68, 4);           // y=4 centres the 24px temp; x=68 over the cleared area
    } else {
      OLED::fillArea(0, 0, 56, 32, 0); // clear the area
      OLED::setCursor(12, 4);          // y=4 centres the 24px temp; x=12 nudges it one digit right
    }
    // If we have a tip connected draw the temp, if not we leave it blank
    if (!tipDisconnectedDisplay) {
      // draw in the temp
      if (!(getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300))) {
        ui_draw_tip_temperature(false, FontStyle::LARGE); // draw in the temp
      }
    } else {
      // Draw in missing tip symbol
      if (OLED::getRotation()) {
        // in right handed mode we want to draw over the first part
        OLED::drawArea(54, 0, 56, 32, disconnectedTipF);
      } else {
        OLED::drawArea(0, 0, 56, 32, disconnectedTip);
      }
    }
  }
}
#endif /* OLED_128x32_DENSE_UI */
#endif /* OLED_128x32_HERO_UI */
#endif
