/**
 * Project: Arcade Controller V1.1
 * File: apps/AppManager.h
 * Description: Owns the InputHandler and all App instances. Routes hardware
 *              input to the currently active App and handles app switching.
 */

#pragma once

#include "App.h"
#include "AppId.h"
#include "../hal/InputHandler.h"

#include "MenuApp/MenuApp.h"
#include "BluetoothApp/BluetoothApp.h"
#include "InfoApp/InfoApp.h"
#include "InputMonitorApp/InputMonitorApp.h"
#include "SpaceInvadersApp/SpaceInvadersApp.h"

class ISystem;

class AppManager {
private:
    InputHandler input;

    // App instances - constructed once, live as long as AppManager.
    MenuApp          menuApp;
    BluetoothApp     bluetoothApp;
    InfoApp          infoApp;
    InputMonitorApp  inputMonitorApp;
    SpaceInvadersApp spaceInvadersApp;

    App*  currentApp   = nullptr;
    AppId currentAppId = AppId::Menu;  // valid only while currentApp != nullptr

public:
    explicit AppManager(ISystem* sys)
        : menuApp(sys),
          bluetoothApp(sys),
          infoApp(sys),
          inputMonitorApp(sys),
          spaceInvadersApp(sys) {}

    void begin();
    void update();

    // --- App switching ---
    void startApp(AppId id);
    bool isCurrent(AppId id) const;

    // Direct accessor for the boot sequence in ArcadeController.
    BluetoothApp* getBluetoothApp() { return &bluetoothApp; }

    // Exposes the InputHandler for apps that poll input directly
    // (InputMonitorApp, SpaceInvadersApp). Event delivery still happens
    // through the registered onEvent callback.
    InputHandler* getInput() { return &input; }

private:
    App* resolveApp(AppId id);
    void handleInput(ControlEvent ev, EventType type);
};
