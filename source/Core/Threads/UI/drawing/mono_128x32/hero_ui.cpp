#include "hero_ui.hpp"
#ifdef OLED_128x32_HERO_UI
#include "OLED.hpp"
#include "SolderingCommon.h"
#include "ui_drawing.hpp"

const HeroFace HeroFaceTall  = {HeroDigitFont, HeroDegreeGlyph, HERO_DIGIT_W, HERO_DIGIT_H, HERO_DEGREE_W, 16};
const HeroFace HeroFaceShort = {HeroShortDigitFont, HeroShortDegreeGlyph, HERO_SHORT_DIGIT_W, HERO_SHORT_DIGIT_H, HERO_SHORT_DEGREE_W, 8};

int16_t hero_x(int16_t x, uint8_t width) { return OLED::getRotation() ? OLED_WIDTH - x - width : x; }

uint8_t hero_digit_count(uint16_t value) {
  uint8_t digits = 1;
  while (value >= 10) {
    value /= 10;
    digits++;
  }
  return digits;
}

uint8_t hero_temperature_width(uint16_t value, const HeroFace &face) { return hero_digit_count(value) * (face.digitW + HERO_DIGIT_GAP) + face.degreeW; }

static void drawGlyph(int16_t x, uint8_t glyph, const HeroFace &face) { OLED::drawArea(x, 0, face.digitW, face.digitH, face.glyphs + glyph * face.digitW * (face.digitH / 8)); }

void ui_draw_hero_temperature(int16_t x, uint16_t value, const HeroFace &face) {
  const uint8_t digits = hero_digit_count(value);
  uint16_t      scale  = 1;
  for (uint8_t i = 1; i < digits; i++) {
    scale *= 10;
  }
  for (; scale > 0; scale /= 10) {
    drawGlyph(x, (value / scale) % 10, face);
    x += face.digitW + HERO_DIGIT_GAP;
  }
  OLED::drawArea(x, 0, face.degreeW, face.degreeH, face.degree);
}

void ui_draw_hero_dashes(int16_t x, uint8_t count, const HeroFace &face) {
  while (count--) {
    drawGlyph(x, HERO_GLYPH_DASH, face);
    x += face.digitW + HERO_DIGIT_GAP;
  }
}

void ui_draw_hero_icon(int16_t x, const uint8_t *icon) { OLED::drawArea(x, 0, HERO_ICON_W, 16, icon); }

void ui_draw_hero_small_icon(int16_t x, int16_t y, const uint8_t *icon) { OLED::drawArea(x, y, HERO_SMALL_ICON_W, 8, icon); }

void ui_draw_hero_live_temperature(int16_t x, const HeroFace &face) {
  if (isTipDisconnected()) {
    ui_draw_hero_dashes(hero_x(x, HERO_NO_TIP_CHARS * (face.digitW + HERO_DIGIT_GAP) - HERO_DIGIT_GAP), HERO_NO_TIP_CHARS, face);
    return;
  }
  const TemperatureType_t reading = getTipTemp();
  const uint16_t          temp    = reading > 0 ? reading : 0;
  ui_draw_hero_temperature(hero_x(x, hero_temperature_width(temp, face)), temp, face);
}

void ui_draw_hero_tip_temperature(int16_t x, TemperatureType_t tipTempC) {
  const bool blinkedOff = tipTempC > 55 && getSettingValue(SettingsOptions::CoolingTempBlink) && (xTaskGetTickCount() % 1000 < 300);
  if (blinkedOff && !isTipDisconnected()) {
    return;
  }
  ui_draw_hero_live_temperature(x);
}

uint8_t tiny_voltage_width(uint32_t voltX10) { return (hero_digit_count(voltX10 / 10) + 3) * TINY_CHAR_W; }

void ui_draw_tiny_voltage(int16_t x, int16_t y, uint32_t voltX10) {
  OLED::setCursor(x, y);
  OLED::printNumber(voltX10 / 10, hero_digit_count(voltX10 / 10), FontStyle::TINY);
  OLED::print(SmallSymbolDot, FontStyle::TINY);
  OLED::printNumber(voltX10 % 10, 1, FontStyle::TINY);
  OLED::print(SmallSymbolVolts, FontStyle::TINY);
}

void ui_draw_tiny_char_2x(int16_t x, int16_t y, uint8_t code) {
  const uint8_t *src = FontSectionInfo.font06_compact_start_ptr + (code - 2) * TINY_CHAR_W;
  uint8_t        out[2][TINY_CHAR_W * 2];
  for (uint8_t col = 0; col < TINY_CHAR_W; col++) {
    uint16_t doubled = 0;
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (src[col] & (1 << bit)) {
        doubled |= 3 << (bit * 2);
      }
    }
    for (uint8_t strip = 0; strip < 2; strip++) {
      out[strip][col * 2] = out[strip][col * 2 + 1] = doubled >> (strip * 8);
    }
  }
  OLED::drawArea(x, y, TINY_CHAR_W * 2, 16, &out[0][0]);
}

#endif /* OLED_128x32_HERO_UI */
