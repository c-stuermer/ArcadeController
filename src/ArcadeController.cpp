/**
 * Project: Arcade Controller V1.2
 * File: ArcadeController.cpp
 */

#include "ArcadeController.h"

void ArcadeController::begin() {
    Serial.begin(115200);
    Serial.println("[SYSTEM] ArcadeController V1.2");

    // 1. HAL singletons
    power.begin();
    display.begin();
    sound.begin();
    settings.begin();

    // 2. App layer (initializes InputHandler and wires its callback to handleInput)
    appManager.begin();

    // 3. Apply persisted display/sound settings
    applySavedSettings();

    // 4. Boot Sequence
    if (!power.isSwitchedOn()) return;

    // The BLE gamepad adapter is owned by BluetoothApp and is only brought
    // up when that app is entered for the first time. This keeps the device
    // off the air whenever the user just wants to use the local apps
    // (Menu, Info, InputMonitor, SpaceInvaders) and saves battery. Once
    // entered, BluetoothApp::stop() does NOT tear the adapter down, so the
    // gamepad keeps working while the user navigates other apps.
    if (settings.getBootMode() == 0) {
        // Stealth Mode: boot straight into BluetoothApp (silent gamepad use
        // case), suppress display + sound output.
        Serial.println("[SYSTEM] Booting in Stealth Mode...");
        display.setBrightness(0);
        sound.setVolume(0);
        appManager.startApp(AppId::Bluetooth);
    } else {
        // Normal Mode: open the menu. BLE only comes up if the user enters
        // BluetoothApp from there.
        Serial.println("[SYSTEM] Booting in Normal Mode...");
        appManager.startApp(AppId::Menu);
        sound.play(SoundEffect::LASER);
    }
}

void ArcadeController::update() {
    // Hardware battery polling
    power.update();

    // --- Power Management ---
    // Physical power switch off -> deep sleep immediately
    if (!power.isSwitchedOn()) {
        Serial.println("[SYSTEM] Switch turned OFF -> Entering Deep Sleep");
        display.setBrightness(0);
        display.clear();
        // Let the display flush the cleared framebuffer before peripherals
        // are powered down in enterDeepSleep() — prevents ghosted pixels.
        delay(50);
        power.enterDeepSleep();
    }

    // --- Battery -> Display sync (every 60s) ---
    // Note: BluetoothApp pulls battery for the gamepad adapter itself,
    // so we no longer push to a gamepad here.
    static uint32_t lastBatteryPush = 0;
    if (millis() - lastBatteryPush > 60000 || lastBatteryPush == 0) {
        display.setBatteryLevel(power.getBatteryPercentage());
        lastBatteryPush = millis();
    }

    // --- App layer ---
    appManager.update();
    sound.update();
}

// --- Cross-cutting setters --------------------------------------------------

void ArcadeController::setBrightness(uint8_t level) {
    settings.setBrightness(level);
    display.setBrightness(level);
}

void ArcadeController::setVolume(uint8_t level) {
    settings.setVolume(level);
    sound.setVolume(level);
}

void ArcadeController::setBootMode(uint8_t mode) {
    settings.setBootMode(mode);
}

// --- Private ----------------------------------------------------------------

void ArcadeController::applySavedSettings() {
    display.setBrightness(settings.getBrightness());
    sound.setVolume(settings.getVolume());
}
