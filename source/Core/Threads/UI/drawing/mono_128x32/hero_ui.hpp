#ifndef UI_DRAWING_HERO_UI_HPP_
#define UI_DRAWING_HERO_UI_HPP_
#include "configuration.h"
#ifdef OLED_128x32_HERO_UI
#include "Types.h"
#include <stdint.h>

constexpr uint8_t HERO_DIGIT_W        = 16;
constexpr uint8_t HERO_DIGIT_H        = 32;
constexpr uint8_t HERO_DIGIT_GAP      = 2;
constexpr uint8_t HERO_DEGREE_W       = 8;
constexpr uint8_t HERO_SHORT_DIGIT_W  = 14;
constexpr uint8_t HERO_SHORT_DIGIT_H  = 24;
constexpr uint8_t HERO_SHORT_DEGREE_W = 6;
constexpr uint8_t HERO_GLYPH_DASH     = 10;
constexpr uint8_t HERO_GLYPH_COUNT    = 11;
constexpr uint8_t HERO_ICON_W         = 8;
constexpr uint8_t HERO_SMALL_ICON_W   = 6;
constexpr uint8_t TINY_CHAR_W         = 6;
constexpr uint8_t HERO_NO_TIP_CHARS   = 3;

extern const uint8_t HeroDigitFont[];
extern const uint8_t HeroDegreeGlyph[];
extern const uint8_t HeroShortDigitFont[];
extern const uint8_t HeroShortDegreeGlyph[];
extern const uint8_t HeroIconBolt[];
extern const uint8_t HeroIconArrowRight[];
extern const uint8_t HeroIconArrowUp[];
extern const uint8_t HeroIconUsbC[];
extern const uint8_t HeroIconBatteryOutline[];
extern const uint8_t HeroIconBoost[];
extern const uint8_t HeroIconHeat[];
extern const uint8_t HeroIconSleep[];

struct HeroFace {
  const uint8_t *glyphs;
  const uint8_t *degree;
  uint8_t        digitW;
  uint8_t        digitH;
  uint8_t        degreeW;
  uint8_t        degreeH;
};
extern const HeroFace HeroFaceTall;
// 24 px cells at y=0, leaving the bottom strip free for a bar.
extern const HeroFace HeroFaceShort;

// Mirrors an element's left edge for left-handed mode, so layouts are written once in right-handed coordinates.
int16_t hero_x(int16_t x, uint8_t width);
uint8_t hero_digit_count(uint16_t value);
uint8_t hero_temperature_width(uint16_t value, const HeroFace &face = HeroFaceTall);
void    ui_draw_hero_temperature(int16_t x, uint16_t value, const HeroFace &face = HeroFaceTall);
void    ui_draw_hero_dashes(int16_t x, uint8_t count, const HeroFace &face = HeroFaceTall);
void    ui_draw_hero_icon(int16_t x, const uint8_t *icon);
// 6x8 icon in a TINY character cell; keep y strip-aligned like TINY text.
void ui_draw_hero_small_icon(int16_t x, int16_t y, const uint8_t *icon);
// Live tip temperature as the hero at right-handed x: dashes without a tip, never blinks.
void ui_draw_hero_live_temperature(int16_t x, const HeroFace &face = HeroFaceTall);
// Live tip temperature as the hero at right-handed x: dashes without a tip, blanked during the cooling blink.
void    ui_draw_hero_tip_temperature(int16_t x, TemperatureType_t tipTempC);
uint8_t tiny_voltage_width(uint32_t voltX10);
void    ui_draw_tiny_voltage(int16_t x, int16_t y, uint32_t voltX10);
// One TINY glyph scaled to 12x16; code is a single-byte font code (digits are 2 + n), y strip-aligned.
void ui_draw_tiny_char_2x(int16_t x, int16_t y, uint8_t code);

#endif /* OLED_128x32_HERO_UI */
#endif /* UI_DRAWING_HERO_UI_HPP_ */
