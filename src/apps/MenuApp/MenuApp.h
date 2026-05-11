/**
 * Project: Arcade Controller V1.0
 * File: MenuApp.h
 * Description: The main system menu application handling categories and dynamic settings.
 */

#pragma once
#include "../App.h"
#include <vector>
#include <functional>
#include <Arduino.h>

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
    // Inherit constructor from App base class
    using App::App; 

    void start() override;
    void update() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    std::vector<MenuCategory> categories;
    int currentCatIndex = 0;
    int currentItemIndex = 0;
    
    // Prevents continuous redrawing of the menu to eliminate display flickering
    bool menuDirty = true; 

    // Internal helper to render the menu items on screen
    void drawMenu();
};