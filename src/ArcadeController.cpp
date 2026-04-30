/**
 * Project: Arcade Controller V0.2
 * File: ArcadeController.cpp
 */

#include "ArcadeController.h"

void ArcadeController::begin() {
    Serial.begin(115200);
    Serial.println("[SYSTEM] ArcadeController V0.2");

    // Initialize all hardware abstraction layers
    power.begin(); 
    display.begin(); 
    sound.begin();
    input.begin(); 
    settings.begin();

    // Initialize the gamepad driver
    gamepadDriver = &bleGamepadAdapter;
    gamepadDriver->begin();

    // Route all hardware input events to the currently active application
    input.onEvent([this](ControlEvent ev, EventType type) {
        this->appManager.handleInput(ev, type);
    });

    // Apply configuration from persistent storage
    applySavedSettings();

    // --- Boot Sequence ---
    if (power.isSwitchedOn()) {
        uint8_t bootMode = settings.getBootMode();

        if (bootMode == 0) {
            // Stealth Mode: Start directly into Bluetooth, disable display and sound
            Serial.println("[SYSTEM] Booting in Stealth Mode...");
            display.setBrightness(0);
            sound.setVolume(0);
            appManager.startApp(&bluetoothApp); 
        } else {
            // Normal Mode: Start into the main menu with startup sound
            Serial.println("[SYSTEM] Booting in Normal Mode...");
            appManager.startApp(&menuApp); 
            sound.play(SoundEffect::LASER);
        }         
    }
}

void ArcadeController::update() {
    // Periodically sync battery state
    syncSystemStats();
    
    // Poll hardware states
    power.update();
    input.update();

    // --- Power Management ---
    // If the physical power switch is turned off, clear the display 
    // and enter deep sleep immediately to preserve battery life.
    if (!power.isSwitchedOn()) {
        Serial.println("[SYTEM] Switch turned OFF -> Entering Deep Sleep");
        display.setBrightness(0);
        display.clear();
        delay(50);
        power.enterDeepSleep();
    }

    // Process active application logic and outputs
    appManager.update();
    sound.update();      
}

// --- Settings Implementation ---

void ArcadeController::applySavedSettings() {
    uint8_t bright = settings.getBrightness();
    display.setBrightness(bright);

    uint8_t vol = settings.getVolume();
    sound.setVolume(vol); 
}

void ArcadeController::updateSystemBrightness(uint8_t level) {
    settings.setBrightness(level); 
    display.setBrightness(level);
}

void ArcadeController::updateSystemVolume(uint8_t level) {
    settings.setVolume(level);
    sound.setVolume(level);
}

void ArcadeController::updateSystemBootMode(uint8_t mode) {
    settings.setBootMode(mode); 
}

void ArcadeController::syncSystemStats() {
    static uint32_t lastBatteryCheck = 0;
    
    // Check battery level every 60 seconds to avoid unnecessary polling overhead
    if (millis() - lastBatteryCheck > 60000 || lastBatteryCheck == 0) {
        int battery = power.getBatteryPercentage();

        // Active push principle: distribute the current battery status to UI and Gamepad
        display.setBatteryLevel(battery);
        gamepadDriver->setBatteryLevel(battery);

        lastBatteryCheck = millis();
        Serial.printf("[SYSTEM] Battery Sync: %d%%\n", battery);
    }
}