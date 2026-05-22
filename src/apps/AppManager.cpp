/**
 * Project: Arcade Controller V1.3
 * File: apps/AppManager.cpp
 */

#include "AppManager.h"

void AppManager::begin() {
    input.onEvent([this](ControlEvent ev, EventType type) {
        this->handleInput(ev, type);
    });

    // Launch the default app so currentApp is never null on the first update().
    if (defaultApp) {
        currentApp = defaultApp;
        currentApp->start();
    }
}

void AppManager::update() {
    input.update();
    if (currentApp) currentApp->update();
}

void AppManager::registerApp(AppId id, App* app, const char* label) {
    if (app) {
        registry[id] = { app, label };
        if (!defaultApp) setDefaultApp(id);
    }
}

void AppManager::setDefaultApp(AppId id) {
    App* nextDefaultApp = resolveApp(id);
    if (!nextDefaultApp) return;
    
    defaultApp = nextDefaultApp;
}

void AppManager::startApp(AppId id) {
    App* next = resolveApp(id);
    if (!next || currentApp == next) return;

    if (currentApp) currentApp->stop();
    currentApp = next;
    currentApp->start();
}

void AppManager::closeApp() {
    if (!defaultApp || currentApp == defaultApp) return;
    
    if (currentApp) currentApp->stop();
    currentApp = defaultApp;
    currentApp->start();
}
void AppManager::switchApp(AppId id) {
    startApp(id);
}

App* AppManager::resolveApp(AppId id) {
    auto it = registry.find(id);
    return (it != registry.end()) ? it->second.app : nullptr;
}

void AppManager::handleInput(ControlEvent ev, EventType type) {
    if (currentApp) currentApp->onInput(ev, type);
}