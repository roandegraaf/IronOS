#ifndef PREVIEW_FIXTURES_H
#define PREVIEW_FIXTURES_H
#include "preview_state.h"
#include <stdint.h>

enum Screen : uint32_t {
  IdleDetailed        = 1 << 0,
  IdleSimplified      = 1 << 1,
  SolderingDetailed   = 1 << 2,
  SolderingSimplified = 1 << 3,
  SleepDetailed       = 1 << 4,
  SleepSimplified     = 1 << 5,
  SettingsMenu        = 1 << 6,
  WarningRunaway      = 1 << 7,
  WarningReset        = 1 << 8,
  WarningUndervoltage = 1 << 9,
};

struct Fixture {
  const char *name;
  uint32_t    screens;
  int32_t     tipC;
  int16_t     handleCx10;
  uint16_t    inputVx10;
  PowerSource source;
  bool        tipDisconnected;
  uint16_t    setpoint;
  uint16_t    boostTemp;
  bool        boost;
  uint32_t    x10Watts;
  uint8_t     heatPercent;
  int32_t     x10WattLimit;
  uint8_t     tipRx10;
  uint16_t    sleepTimeSetting; // SettingsOptions::SleepTime: 1-5 = x10 s, 6+ = (n-5) min, 0 = off
  uint32_t    msSinceMovement;
  uint16_t    settingsMainEntry;
  uint16_t    settingsSubEntry; // 0 = category page, n = nth item inside it
  uint8_t     dcCells;          // SettingsOptions::MinDCVoltageCells: 0 = plain DC, n = (n + 2)S lithium pack
};

// clang-format off
static const Fixture kFixtures[] = {
  // name                  screens                                    tipC handle  Vx10 source           noTip  set  boost boostOn  W x10 heat% limit Rx10 sleep msSinceMove menu sub cells
  {"idle_cold_set320",     IdleDetailed | IdleSimplified,               24,  240,  120, PowerSource::DC,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_pd_handle31",     IdleDetailed | IdleSimplified,               26,  310,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_cooling_180",     IdleDetailed | IdleSimplified,              180,  330,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_no_tip",          IdleDetailed | IdleSimplified,                0,  250,  201, PowerSource::PD,  true,  320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_hot_dc_450",      IdleDetailed | IdleSimplified,              448,  420,  120, PowerSource::DC,  false, 450, 450, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_wide_28v",        IdleDetailed | IdleSimplified,              388,  400,  280, PowerSource::PD,  false, 400, 450, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
  {"idle_battery_4s",      IdleDetailed | IdleSimplified,               24,  240,  152, PowerSource::DC,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    2},
  {"solder_318_heat42_boost", SolderingDetailed | SolderingSimplified, 318,  350,  201, PowerSource::PD,  false, 320, 420, true,    252,  42,  600,  62,    5,     2000,      0,   0,    0},
  {"solder_45w_sleeptimer", SolderingDetailed | SolderingSimplified,   320,  360,  201, PowerSource::PD,  false, 320, 420, false,   450,  75,  600,  62,    5,     8000,      0,   0,    0},
  {"solder_overshoot_heat0", SolderingDetailed | SolderingSimplified,  324,  380,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,     2000,      0,   0,    0},
  {"solder_288_heat100_boost450", SolderingDetailed | SolderingSimplified, 288, 380, 201, PowerSource::PD, false, 320, 450, true,    650, 100,  600,  62,    5,     2000,      0,   0,    0},
  {"solder_warmup_64",     SolderingDetailed | SolderingSimplified,      64,  260,  201, PowerSource::PD,  false, 320, 420, false,   650, 100,  600,  62,    5,     1000,      0,   0,    0},
  {"solder_no_tip",        SolderingDetailed | SolderingSimplified,       0,  260,  201, PowerSource::PD,  true,  320, 420, false,     0,   0,  600,  62,    5,     1000,      0,   0,    0},
  {"solder_120w_28v",      SolderingDetailed,                           330,  400,  280, PowerSource::PD,  false, 330, 420, false,  1205,  90, 1400,  62,    5,     2000,      0,   0,    0},
  {"sleep_150",            SleepDetailed | SleepSimplified,            150,  320,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,    60000,      0,   0,    0},
  {"settings_soldering",   SettingsMenu,                                24,  240,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      1,   0,    0},
  {"settings_boost_temp",  SettingsMenu,                                24,  240,  201, PowerSource::PD,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      1,   1,    0},
  {"warnings",             WarningRunaway | WarningReset | WarningUndervoltage, 24, 240,  52, PowerSource::DC,  false, 320, 420, false,     0,   0,  600,  62,    5,        0,      0,   0,    0},
};
// clang-format on

#endif
