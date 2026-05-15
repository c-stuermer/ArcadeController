/**
 * Project: Arcade Controller V1.1
 * File: ArcadeController.h
 * Description: Composition root and ISystem implementation.
 *
 * Owns only the HAL singletons and the AppManager. Knows nothing about
 * concrete apps or the gamepad transport.
 */

#pragma once
#include <Arduino.h>

#include "config/Config.h"
#include "apps/ISystem.h"

#include "hal/SettingsManager.h"
#include "hal/PowerManager.h"
#include "hal/DisplayManager.h"
#include "hal/SoundManager.h"

#include "apps/AppManager.h"

class ArcadeController : public ISystem {
private:
    PowerManager    power;
    DisplayManager  display;
    SoundManager    sound;
    SettingsManager settings;

    AppManager appManager;

public:
    ArcadeController() : appManager(this) {}

    void begin();
    void update();

    // --- ISystem implementation -------------------------------------------
    DisplayManager*  getDisplay()    override { return &display; }
    SoundManager*    getSound()      override { return &sound; }
    PowerManager*    getPower()      override { return &power; }
    SettingsManager* getSettings()   override { return &settings; }
    AppManager*      getAppManager() override { return &appManager; }
    InputHandler*    getInput()      override { return appManager.getInput(); }

    void setBrightness(uint8_t level) override;
    void setVolume(uint8_t level)     override;
    void setBootMode(uint8_t mode)    override;
    // ----------------------------------------------------------------------

private:
    void applySavedSettings();
};
