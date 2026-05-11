/**
 * Project: Arcade Controller V1.0
 * File: SettingsManager.h
 * Description: Manages persistent storage of user settings using ESP32 Preferences.
 */

#pragma once
#include <Arduino.h>
#include <Preferences.h>

class SettingsManager {
private:
    Preferences prefs;

    // Cached values for fast access without flash reads
    uint8_t _volume;
    uint8_t _brightness;
    uint8_t _bootMode;
    uint8_t _gamepadMode;

public:
    // Initializes the preferences and loads stored values
    void begin();

    // --- Getters ---
    uint8_t getBootMode() const    { return _bootMode; }
    uint8_t getVolume() const      { return _volume; }
    uint8_t getBrightness() const  { return _brightness; }
    uint8_t getGamepadMode() const { return _gamepadMode; }

    // --- Setters (Save to Flash) ---
    void setBootMode(uint8_t mode);
    void setVolume(uint8_t level);
    void setBrightness(uint8_t level);
    void setGamepadMode(uint8_t mode);
};