/**
 * Project: Arcade Controller V1.3
 * File: SystemManager.cpp
 */

#include "SystemManager.h"

void SystemManager::begin() {
    Serial.begin(115200);
    Serial.println("[SYSTEM] SystemManager V1.3 initializing...");

    // 1. Initialize hardware on the lowest level
    power.begin();
    display.begin();
    sound.begin();
    settings.begin();
    inputReader.begin();

    applySavedSettings();
    display.setBatteryLevel(power.getBatteryPercentage());

    // 2. Safety check: Catch edge case where the switch was toggled
    // too fast during the boot sequence.
    if (!power.isSwitchedOn()) {
        return;
    }

    // 3. Configure physical hardware state based on boot mode settings
    if (settings.getBootMode() == 0) {
        Serial.println("[SYSTEM] Hardware booting in Stealth Mode...");
        display.setBrightness(0);
        sound.setVolume(0);
    } else {
        Serial.println("[SYSTEM] Hardware booting in Normal Mode...");
        sound.play(SoundEffect::LASER);
    }
}

void SystemManager::update() {
    // Poll the low-level battery state
    power.update();

    // Sync battery level to the display header every 10 seconds
    syncBatteryToDisplay();

    // Drive ongoing sound-effect synthesis and automatic stop after duration
    sound.update();

    // Watchdog for the physical power switch
    if (!power.isSwitchedOn()) {
        initiateShutdown();
        return;
    }
}

void SystemManager::initiateShutdown() {
    Serial.println("[SYSTEM] Switch turned OFF -> Preparing Deep Sleep");

    // Stop any active sound effect, then mute and clear the display
    sound.stop();
    display.setBrightness(0);
    display.clear();

    // Allow SPI TFT framebuffer to flush completely before cutting power
    delay(50);

    // Enter deep sleep state via PowerManager
    power.enterDeepSleep();
}

void SystemManager::syncBatteryToDisplay() {
    const unsigned long now = millis();
    if (now - lastBatteryDisplaySync >= BATTERY_DISPLAY_SYNC_MS) {
        display.setBatteryLevel(power.getBatteryPercentage());
        lastBatteryDisplaySync = now;
    }
}

void SystemManager::applySavedSettings() {
    display.setBrightness(settings.getBrightness());
    sound.setVolume(settings.getVolume());
}