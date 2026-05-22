/**
 * Project: Arcade Controller V1.3
 * File: SystemManager.h
 * Description: System lifecycle manager. Owns the hardware subsystems directly, 
 * handles hardware initialization, deep sleep routing, and exposes abstract 
 * interfaces to the application layer.
 */

#pragma once
#include <Arduino.h>

// Concrete HAL classes (SystemManager owns these)
#include "hal/PowerManager.h"
#include "hal/DisplayManager.h"
#include "hal/SoundManager.h"
#include "hal/SettingsManager.h"
#include "hal/InputReader.h"

// Interfaces (Returned to the AppManager/Apps)
#include "hal/interfaces/IDisplay.h"
#include "hal/interfaces/ISound.h"
#include "hal/interfaces/IPower.h"
#include "hal/interfaces/ISetting.h"
#include "hal/interfaces/IInputReader.h"

class SystemManager {
public:
    // SettingsManager requires display and sound references for cross-cutting updates
    SystemManager() : settings(&display, &sound) {}

    void begin();
    void update();

    // --- Interface Providers for Application Layer ---
    IDisplay* getDisplay()     { return &display; }
    ISound* getSound()       { return &sound; }
    IPower* getPower()       { return &power; }
    ISetting* getSettings()    { return &settings; }
    IInputReader* getInputReader() { return &inputReader; }

private:
    // Concrete hardware instances (Composition)
    PowerManager    power;
    DisplayManager  display;
    SoundManager    sound;
    SettingsManager settings;
    InputReader     inputReader;

    void applySavedSettings();
    void initiateShutdown();
    void syncBatteryToDisplay();

    static constexpr unsigned long BATTERY_DISPLAY_SYNC_MS = 10000;
    unsigned long lastBatteryDisplaySync = 0;
};