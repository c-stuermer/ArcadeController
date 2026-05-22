/**
 * Project: Arcade Controller V1.3
 * File: apps/AppManager.h
 */

#pragma once

#include <map>

#include "App.h"
#include "AppId.h"
#include "interfaces/IAppNavigator.h"
#include "../hal/InputHandler.h"

// Inherit from IAppNavigator to achieve loose coupling with the app layer
class AppManager : public IAppNavigator {
public:
    explicit AppManager(InputHandler& handler) : input(handler) {}

    void begin();
    void update();

    // System/Composition Root API
    // label is optional: pass a string to show the app in the auto-generated
    // APPLICATIONS menu, or omit/nullptr to keep it out of the menu.
    void registerApp(AppId id, App* app, const char* label = nullptr);
    void setDefaultApp(AppId id);
    void startApp(AppId id);

    // Interface Implementation (App Layer API)
    void closeApp() override;
    void switchApp(AppId id) override;

    InputHandler* getInput() { return &input; }

    // Read-only access to the registry for the menu auto-generation loop.
    const std::map<AppId, AppEntry>& getRegistry() const override { return registry; }

private:
    InputHandler&              input;     // non-owning — constructed and owned by main.cpp
    std::map<AppId, AppEntry>  registry;
    App* currentApp = nullptr;
    App* defaultApp = nullptr;

    App* resolveApp(AppId id);
    void handleInput(ControlEvent ev, EventType type);
};