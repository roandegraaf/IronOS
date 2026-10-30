#include "OLED.hpp"
#include "OperatingModes.h"
#include "Settings.h"
#include "fixtures.h"
#include "power.hpp"
#include "preview_state.h"
#include "ui_drawing.hpp"

extern TickType_t lastMovementTime;

static const TickType_t kTick = 600500; // tick % 1000 >= 300 keeps blinking temps visible; tick % 250 < 125 shows the menu scrollbar

static void applyFixture(const Fixture &f, bool leftHanded) {
  previewState.tick            = kTick;
  previewState.tipC            = f.tipC;
  previewState.handleCx10      = f.handleCx10;
  previewState.inputVx10       = f.inputVx10;
  previewState.source          = f.source;
  previewState.tipDisconnected = f.tipDisconnected;
  previewState.heaterPWM       = (f.heatPercent * 255) / 100;
  previewState.x10WattLimit    = f.x10WattLimit;
  previewState.tipRx10         = f.tipRx10;
  x10WattHistory.sum           = f.x10Watts;
  lastMovementTime             = kTick - f.msSinceMovement;
  lastButtonTime               = (f.screens & SettingsMenu) ? kTick : lastMovementTime;

  resetSettings();
  setSettingValue(SettingsOptions::SolderingTemp, f.setpoint);
  setSettingValue(SettingsOptions::BoostTemp, f.boostTemp);
  setSettingValue(SettingsOptions::SleepTime, f.sleepTimeSetting);
  setSettingValue(SettingsOptions::TemperatureInF, 0);
  setSettingValue(SettingsOptions::MinDCVoltageCells, f.dcCells);
  setSettingValue(SettingsOptions::OrientationMode, leftHanded ? orientationMode_t::LEFT : orientationMode_t::RIGHT);
}

static void drawScreen(const Fixture &f, Screen screen) {
  OLED::clearScreen();
  switch (screen) {
  case IdleDetailed:
  case IdleSimplified:
    OLED::setCursor(OLED::getRotation() ? 50 : -1, 0);
    if (screen == IdleDetailed) {
      ui_draw_homescreen_detailed(f.tipC);
    } else {
      ui_draw_homescreen_simplified(f.tipC);
    }
    break;
  case SolderingDetailed:
    ui_draw_soldering_power_status(f.boost);
    break;
  case SolderingSimplified:
    ui_draw_soldering_basic_status(f.boost);
    break;
  case SleepDetailed:
    ui_draw_soldering_detailed_sleep(f.tipC);
    break;
  case SleepSimplified:
    ui_draw_soldering_basic_sleep(f.tipC);
    break;
  case SettingsMenu: {
    guiContext cxt           = {};
    cxt.viewEnterTime        = kTick;
    cxt.scratch_state.state1 = f.settingsMainEntry;
    cxt.scratch_state.state2 = f.settingsSubEntry;
    gui_SettingsMenu(BUTTON_NONE, &cxt);
    break;
  }
  case WarningRunaway:
    warnUser(translatedString(Tr->WarningThermalRunaway), BUTTON_NONE);
    break;
  case WarningReset:
    warnUser(translatedString(Tr->SettingsResetMessage), BUTTON_NONE);
    break;
  case WarningUndervoltage:
    ui_draw_warning_undervoltage();
    break;
  }
  OLED::refresh();
}

void renderFixture(const Fixture &f, Screen screen, bool leftHanded) {
  OLED::initialize();
  applyFixture(f, leftHanded);
  OLED::setRotation(leftHanded);
  ui_pre_render_assets();
  drawScreen(f, screen);
}
