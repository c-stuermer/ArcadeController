/**
 * Project: Arcade Controller V0.2
 * File: AppManager.h
 * Description: Manages the lifecycle and switching of the active application.
 * Note: Consider letting AppManager own the App instances instead of ArcadeController in the future.
 */

#pragma once
#include "App.h"

class AppManager {
private:
    App* currentApp = nullptr;

public:
    // Switches to a new application, handling the lifecycle (stop old -> start new)
    void startApp(App* newApp) {
        if (currentApp) {
            currentApp->stop();
        }

        currentApp = newApp;
        
        if (currentApp) {
            currentApp->start();
        }
    }

    // Called in the main loop to process the active application
    void update() {
        if (currentApp) currentApp->update();
    }

    // Routes input events to the currently active app
    void handleInput(ControlEvent ev, EventType type) {
        if (currentApp) currentApp->onInput(ev, type);
    }

    // Helper to check which app is currently running
    bool isCurrent(App* app) {
        return currentApp == app;
    }
};