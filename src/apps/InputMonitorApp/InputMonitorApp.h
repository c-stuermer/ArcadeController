/**
 * Project: Arcade Controller V1.2
 * File: InputMonitorApp.h
 * Description: Graphical tool to visualize controller inputs and latency.
 */

#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../apps/App.h"

class InputMonitorApp : public App {
public:
    using App::App;

    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    // Sentinel: all bits set forces a full redraw on the first update(),
    // because every bit is treated as "changed" against this value.
    static constexpr uint16_t ALL_INPUTS_DIRTY = 0xFFFFu;

    // Cached debounced-state bitmap of the previous frame. Bit positions
    // mirror the ControlEvent enum (see InputHandler::getDebouncedStates()).
    uint16_t lastState = ALL_INPUTS_DIRTY;

    void drawArcadeBtn(TFT_eSPI* gfx, int x, int y, int r, uint16_t color, bool pressed);
    void drawJoystick(TFT_eSPI* gfx, int baseX, int baseY, bool up, bool down, bool left, bool right);
};
