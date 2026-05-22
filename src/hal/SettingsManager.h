/**
 * Project: Arcade Controller V1.3
 * File: SettingsManager.h
 * Description: Manages persistent storage of user settings using ESP32
 *              Preferences. Implements the ISetting interface.
 *
 *              V1.3 change: SettingsManager now observes IDisplay and
 *              ISound. Calls to setBrightness()/setVolume() persist the
 *              value AND apply it to the corresponding subsystem in a
 *              single call. This replaces the cross-cutting setters that
 *              previously lived on ISystem and lets apps depend on
 *              ISetting alone for these adjustments.
 */

#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "interfaces/ISetting.h"
#include "interfaces/IDisplay.h"
#include "interfaces/ISound.h"

class SettingsManager : public ISetting {
public:
    // Cross-cutting setters need display + sound to apply the new value
    // alongside persisting it. Both pointers are non-owning.
    SettingsManager(IDisplay* display, ISound* sound)
        : display(display), sound(sound) {}

    // Initializes the preferences and loads stored values
    void begin();

    // --- Getters ---
    uint8_t getBootMode()    const override { return bootMode; }
    uint8_t getVolume()      const override { return volume; }
    uint8_t getBrightness()  const override { return brightness; }
    uint8_t getGamepadMode() const override { return gamepadMode; }

    // --- Setters (persist to flash AND apply where applicable) ---
    void setBootMode(uint8_t mode)    override;
    void setVolume(uint8_t level)     override;
    void setBrightness(uint8_t level) override;
    void setGamepadMode(uint8_t mode) override;

private:
    Preferences prefs;

    // Observed subsystems (non-owning). setBrightness/setVolume route
    // through these so a single ISetting call updates flash AND hardware.
    IDisplay* display;
    ISound*   sound;

    // Cached values for fast access without flash reads
    uint8_t volume      = 100;
    uint8_t brightness  = 100;
    uint8_t bootMode    = 0;
    uint8_t gamepadMode = 0;
};
