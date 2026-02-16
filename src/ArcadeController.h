/**
 * Project: Arcade Controller V0.1
 * File: ArcadeController.h
 * Description: Main controller class coordinating Hardware (HAL) and Software (Apps).
 */

#pragma once

#include <Arduino.h>

// HAL (Hardware Abstraction Layer) Includes
#include "hal/PowerManager.h"
#include "hal/DisplayManager.h"
#include "hal/InputHandler.h"
#include "hal/SoundManager.h"
#include "config/Config.h"

// App Management
#include "apps/AppManager.h"
#include "apps/InputMonitorApp/InputMonitorApp.h"
#include "apps/RechargeApp/RechargeApp.h"
//#include "apps/MenuApp/menuApp.h"

// Driver Selection Configuration
// Uncomment the following line to use real Bluetooth hardware.
// Comment it out to use the Dummy/Simulation driver (e.g., for debugging without BLE stack).
// #define USE_REAL_BLUETOOTH 

#ifdef USE_REAL_BLUETOOTH
    #include "BleGamepadAdapter.h"
#else
    #include "transport/DummyGamepad.h"
#endif

class ArcadeController {
    private:
        // --- Hardware Managers ---
        PowerManager power;
        DisplayManager display;
        InputHandler input;
        SoundManager sound;
        
        // --- BLE / Gamepad Driver ---
        IGamepadOutput* gamepadDriver;
        
        #ifdef USE_REAL_BLUETOOTH
            BleGamepadAdapter realGamepad;
        #else
            DummyGamepad dummyGamepad;
        #endif

        // --- Software / App Management ---
        AppManager appManager;
        
        // App Instances (injected with 'this' controller context)
        InputMonitorApp inputMonitorApp{this};
        RechargeApp rechargeApp{this};
        // MenuApp menuApp;

    public:
        void begin();
        void update();

        // --- API for Apps (Getters) ---
        // Provides apps access to hardware subsystems
        DisplayManager* getDisplay()    { return &display; }
        PowerManager* getPower()      { return &power; }
        InputHandler* getInput()      { return &input; }
        SoundManager* getSound()      { return &sound; }
        IGamepadOutput* getGamepad()    { return gamepadDriver; }
        AppManager* getAppManager() { return &appManager; }

        // Shortcut to switch apps
        void startApp(App* app) { appManager.startApp(app); }

        // Access specific app instances
        App* getInputMonitorApp() { return &inputMonitorApp; }
};