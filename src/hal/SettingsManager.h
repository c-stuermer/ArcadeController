/**
 * Project: Arcade Controller V1.2
 * File: SettingsManager.h
 * Description: Manages persistent storage of user settings using ESP32 Preferences.
 */

#pragma once
#include <Arduino.h>
#include <Preferences.h>

class SettingsManager {
public:
    // Initializes the preferences and loads stored values
    void begin();

    // --- Getters ---
    uint8_t getBootMode() const    { return bootMode; }
    uint8_t getVolume() const      { return volume; }
    uint8_t getBrightness() const  { return brightness; }
    uint8_t getGamepadMode() const { return gamepadMode; }

    // --- Setters (Save to Flash) ---
    void setBootMode(uint8_t mode);
    void setVolume(uint8_t level);
    void setBrightness(uint8_t level);
    void setGamepadMode(uint8_t mode);

private:
    Preferences prefs;

    // Cached values for fast access without flash reads
    uint8_t volume      = 100;
    uint8_t brightness  = 100;
    uint8_t bootMode    = 0;
    uint8_t gamepadMode = 0;
};
