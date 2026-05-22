/**
 * Project: Arcade Controller V1.3
 * File: ISetting.h
 * Description: Abstract interface for the persistent-settings subsystem.
 *              Defines the contract that any concrete SettingsManager
 *              implementation must fulfil.
 */

#pragma once
#include <Arduino.h>

class ISetting {
public:
    virtual ~ISetting() = default;

    // --- Getters ---
    virtual uint8_t getBootMode()    const        = 0;
    virtual uint8_t getVolume()      const        = 0;
    virtual uint8_t getBrightness()  const        = 0;
    virtual uint8_t getGamepadMode() const        = 0;

    // --- Setters (persist to flash) ---
    virtual void setBootMode(uint8_t mode)        = 0;
    virtual void setVolume(uint8_t level)         = 0;
    virtual void setBrightness(uint8_t level)     = 0;
    virtual void setGamepadMode(uint8_t mode)     = 0;
};
