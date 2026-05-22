/**
 * Project: Arcade Controller V1.3
 * File: apps/interfaces/IAppNavigator.h
 * Description: Interface for app navigation and lifecycle control.
 * Exposes only the navigation methods required by the apps.
 *
 * AppEntry lives here so any consumer of IAppNavigator can iterate
 * the registry without depending on the concrete AppManager.
 */

#pragma once
#include <map>
#include "../AppId.h"
#include "../App.h"

// Pairing of an App instance with its optional menu label.
// label == nullptr means the app is not listed in the auto-generated menu.
struct AppEntry {
    App*        app;
    const char* label;   // display name for the APPLICATIONS menu, or nullptr
};

class IAppNavigator {
public:
    virtual ~IAppNavigator() = default;

    // Closes the active app and returns to the designated default app
    virtual void closeApp() = 0;

    // Performs a direct transition to the specified app
    virtual void switchApp(AppId id) = 0;

    // Read-only registry — used by MenuApp to auto-generate the app list
    virtual const std::map<AppId, AppEntry>& getRegistry() const = 0;
};