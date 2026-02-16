/**
 * Project: Arcade Controller V0.1
 * File: InputMonitorApp.h
 * Description: Visualizes gamepad inputs on the OLED display.
 */

#pragma once
#include "../App.h"

// Forward declarations
class ArcadeController; 
class U8G2;

class InputMonitorApp : public App {
public:
    // Inherit constructor from base class
    using App::App;

    // Lifecycle methods
    void start() override;
    void update() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    // Helper to draw a UI button representation
    void drawBtn(U8G2* gfx, int x, int y, int r, const char* label, bool active);
};