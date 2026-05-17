/**
 * Project: Arcade Controller V1.2
 * File: SettingsManager.cpp
 * Description: Implementation of persistent settings logic.
 */

#include "SettingsManager.h"

void SettingsManager::begin() {
    // Open the "arcade" namespace in read/write mode (false)
    prefs.begin("arcade", false);

    // Load values. If the key doesn't exist yet, use the provided default.
    volume      = prefs.getUChar("volume",      100);
    brightness  = prefs.getUChar("brightness",  100);
    bootMode    = prefs.getUChar("bootMode",    0);
    gamepadMode = prefs.getUChar("gamepadMode", 0);  // 0 = BLE_HID by default
}

void SettingsManager::setVolume(uint8_t level) {
    volume = level;
    prefs.putUChar("volume", volume);
}

void SettingsManager::setBrightness(uint8_t level) {
    brightness = level;
    prefs.putUChar("brightness", brightness);
}

void SettingsManager::setBootMode(uint8_t mode) {
    bootMode = mode;
    prefs.putUChar("bootMode", bootMode);
}

void SettingsManager::setGamepadMode(uint8_t mode) {
    gamepadMode = mode;
    prefs.putUChar("gamepadMode", gamepadMode);
}
