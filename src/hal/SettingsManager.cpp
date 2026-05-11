/**
 * Project: Arcade Controller V1.0
 * File: SettingsManager.cpp
 * Description: Implementation of persistent settings logic.
 */

#include "SettingsManager.h"

void SettingsManager::begin() {
    // Open the "arcade" namespace in read/write mode (false)
    prefs.begin("arcade", false);

    // Load values. If the key doesn't exist yet, use the provided default (e.g., 100 or 0)
    _volume = prefs.getUChar("volume", 100);
    _brightness = prefs.getUChar("brightness", 100);
    _bootMode = prefs.getUChar("bootMode", 0);
    _gamepadMode = prefs.getUChar("gamepadMode", 0); // 0 = BLE_HID by default
}

void SettingsManager::setVolume(uint8_t level) {
    _volume = level;
    // Save permanently to flash storage
    prefs.putUChar("volume", _volume); 
}

void SettingsManager::setBrightness(uint8_t level) {
    _brightness = level;
    // Save permanently to flash storage
    prefs.putUChar("brightness", _brightness); 
}

void SettingsManager::setBootMode(uint8_t mode) {
    _bootMode = mode;
    // Save permanently to flash storage
    prefs.putUChar("bootMode", _bootMode);
}

void SettingsManager::setGamepadMode(uint8_t mode) {
    _gamepadMode = mode;
    // Save permanently to flash storage
    prefs.putUChar("gamepadMode", _gamepadMode);
}