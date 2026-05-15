/**
 * Project: Arcade Controller V1.1
 * File: apps/AppManager.cpp
 */

#include "AppManager.h"

void AppManager::begin() {
    input.begin();

    // Route all hardware input events to the currently active app.
    input.onEvent([this](ControlEvent ev, EventType type) {
        this->handleInput(ev, type);
    });
}

void AppManager::update() {
    input.update();
    if (currentApp) currentApp->update();
}

void AppManager::startApp(AppId id) {
    App* next = resolveApp(id);
    if (!next) return;

    // No-op if the same app is already running
    if (currentApp == next) return;

    if (currentApp) currentApp->stop();
    currentApp   = next;
    currentAppId = id;
    currentApp->start();
}

bool AppManager::isCurrent(AppId id) const {
    return currentApp != nullptr && currentAppId == id;
}

App* AppManager::resolveApp(AppId id) {
    switch (id) {
        case AppId::Menu:          return &menuApp;
        case AppId::Bluetooth:     return &bluetoothApp;
        case AppId::Info:          return &infoApp;
        case AppId::InputMonitor:  return &inputMonitorApp;
        case AppId::SpaceInvaders: return &spaceInvadersApp;
    }
    return nullptr;
}

void AppManager::handleInput(ControlEvent ev, EventType type) {
    if (currentApp) currentApp->onInput(ev, type);
}
