/**
 * Project: Arcade Controller V1.3
 * File: MenuApp.h
 * Description: The main system menu application handling categories and
 *              dynamic settings.
 *
 *              V1.3 architecture change:
 *                MenuApp no longer takes an ISystem*. It is constructed
 *                in the composition root and receives only the narrow
 *                interfaces it actually uses (IDisplay, ISound, ISetting)
 *                plus the AppManager pointer for app switching. There is
 *                no path from MenuApp to the concrete DisplayManager or
 *                its rendering backend.
 */

#pragma once
#include "../App.h"
#include "../AppId.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/ISound.h"
#include "../../hal/interfaces/ISetting.h"
#include <vector>
#include <functional>
#include <Arduino.h>

#include "../interfaces/IAppNavigator.h"

struct MenuItem {
    const char* name;
    std::function<void()> action;
    std::function<String()> getValue;

    // Constructor 1: Standard menu item (no dynamic value display)
    MenuItem(const char* n, std::function<void()> act)
        : name(n), action(act), getValue(nullptr) {}

    // Constructor 2: Settings menu item (with dynamic value display like "ON" or "75%")
    MenuItem(const char* n, std::function<void()> act, std::function<String()> val)
        : name(n), action(act), getValue(val) {}
};

struct MenuCategory {
    const char* title;
    std::vector<MenuItem> items;
};

class MenuApp : public App {
public:
    MenuApp(IDisplay*      display,
            ISound*        sound,
            ISetting*      settings,
            IAppNavigator* appNavigator);

    void start() override;
    void update() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    // Injected subsystems (non-owning). MenuApp sees only the interfaces
    // — no path to the concrete DisplayManager or to TFT_eSPI.
    IDisplay*   display;
    ISound*     sound;
    ISetting*   settings;
    IAppNavigator* appManager;

    std::vector<MenuCategory> categories;
    int currentCatIndex  = 0;
    int currentItemIndex = 0;

    // Prevents continuous redrawing of the menu to eliminate display flickering
    bool menuDirty = true;

    // Internal helper to render the menu items on screen
    void drawMenu();
};
