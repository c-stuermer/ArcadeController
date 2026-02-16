/**
 * Project: Arcade Controller V0.1
 * File: ArcadeController.cpp
 * Description: Implementation of the main control logic.
 */

#include "ArcadeController.h"

void ArcadeController::begin() {
    Serial.begin(115200);
    Serial.println("ArcadeController V0.1 (I2C Display Version) starting...");

    // 1. Initialize Hardware
    power.init(); 
    display.init(); 
    // settings.begin(); // TODO: Add persistent settings storage later
    
    // 2. Select and Initialize Gamepad Driver
    #ifdef USE_REAL_BLUETOOTH
        gamepadDriver = &realGamepad;
    #else
        gamepadDriver = &dummyGamepad;
        Serial.println("WARNING: Running in SIMULATION mode (DummyGamepad, no Bluetooth)");
    #endif
    gamepadDriver->begin();

    // 3. Initialize Input
    input.init(); 

    // 4. Configure Input Routing
    // Inputs are routed to the active App via the AppManager
    input.onEvent([this](ControlEvent ev, EventType type) {
        this->appManager.handleInput(ev, type);
    });

    // 5. Initial App Decision based on Power State
    if (power.isSwitchedOn()) {
        // Hardware switch is ON -> Start main functionality
        appManager.startApp(&inputMonitorApp); 
    } else {
        // Hardware switch is OFF -> System woke up due to USB/Charging
        appManager.startApp(&rechargeApp);
    }

    // 6. Startup Feedback
    sound.begin();
    sound.play(SoundEffect::STARTUP);
}

void ArcadeController::update() {
    // --- POWER MANAGEMENT HANDLING ---
    power.update();

// Check if the physical power switch is OFF
    if (!power.isSwitchedOn()) {
        // Simplified Logic: 
        // If switch is OFF, always ensure RechargeApp is running.
        // We do not check for USB connection for now.
        if (!appManager.isCurrent(&rechargeApp)) {
            appManager.startApp(&rechargeApp);
        }
    }

    // --- MAIN LOOP UPDATE ---
    input.update(gamepadDriver);
    appManager.update();
    sound.update();
    
    // Note: I2C Display update might introduce latency (~38ms framerate limit)
    display.show();
}