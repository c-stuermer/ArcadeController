/**
 * Project: Arcade Controller V0.2
 * File: ArcadeController.h
 * Description: Main controller class coordinating Hardware (HAL) and Software (Apps).
 */

#pragma once

#include <Arduino.h>

// --- Configuration ---
#include "config/Config.h"

// --- HAL (Hardware Abstraction Layer) ---
#include "hal/SettingsManager.h"
#include "hal/PowerManager.h"
#include "hal/DisplayManager.h"
#include "hal/InputHandler.h"
#include "hal/SoundManager.h"

// --- Transport / Gamepad ---
#include "transport/IGamepadOutput.h"
#include "transport/BLEGamepadAdapter.h"

// --- Applications ---
#include "apps/AppManager.h"
#include "apps/InputMonitorApp/InputMonitorApp.h"
#include "apps/BluetoothApp/BluetoothApp.h"
#include "apps/MenuApp/MenuApp.h"
#include "apps/InfoApp/InfoApp.h"

class ArcadeController {
    private:
        // --- Hardware Managers ---
        PowerManager power;
        DisplayManager display;
        InputHandler input;
        SoundManager sound;
        SettingsManager settings;
        
        // --- Bluetooth / Gamepad Driver ---
        IGamepadOutput* gamepadDriver;
        BleGamepadAdapter bleGamepadAdapter;

        // --- Software / App Management ---
        AppManager appManager;
        
        // --- App Instances ---
        InputMonitorApp inputMonitorApp{this};
        BluetoothApp bluetoothApp{this};
        MenuApp menuApp{this};
        InfoApp infoApp{this};

    public:

        //Initializes all hardware components, loads settings, and boots the system.
        void begin();

        // Main processing loop. Handles hardware updates, power state, and app logic.
        void update();

        // --- API for Apps (Getters) ---
        DisplayManager* getDisplay()    { return &display; }
        PowerManager* getPower()        { return &power; }
        InputHandler* getInput()        { return &input; }
        SoundManager* getSound()        { return &sound; }
        IGamepadOutput* getGamepad()    { return gamepadDriver; }
        AppManager* getAppManager()     { return &appManager; }
        SettingsManager* getSettings()  { return &settings; }

        // Shortcut to switch the currently active application.
        void startApp(App* app) { appManager.startApp(app); }

        // --- Access Specific App Instances ---
        App* getInputMonitorApp() { return &inputMonitorApp; }
        App* getBluetoothApp()    { return &bluetoothApp; }
        App* getMenuApp()         { return &menuApp; }
        App* getInfoApp()         { return &infoApp; }
        
        // --- System Settings Management ---
        void updateSystemBrightness(uint8_t level);
        void updateSystemVolume(uint8_t level);
        void updateSystemBootMode(uint8_t mode);
        
        //Applies settings (like brightness and volume) loaded from persistent storage.
        void applySavedSettings();  


        // Synchronizes system statistics (e.g., battery level) across relevant components.
        void syncSystemStats();
};