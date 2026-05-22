/**
 * Project: Arcade Controller V1.3
 * File: InputMonitorApp.h
 * Description: Graphical tool to visualize controller inputs and latency.
 *
 *              V1.3: interface-injected, no ISystem dependency. The
 *              V1.2 TFT_eSprite-based anti-flicker drawing was removed
 *              when the rendering backend was pushed behind IDisplay —
 *              sprites are TFT_eSPI-specific and don't survive the
 *              abstraction. Per-frame redraws are limited to the
 *              actually-changed inputs (same strategy as V1.2), so the
 *              visible flicker stays minimal.
 */

#pragma once
#include <Arduino.h>
#include "../App.h"
#include "../AppId.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/IInputH.h"

#include "../interfaces/IAppNavigator.h"

class InputMonitorApp : public App {
public:
    InputMonitorApp(IDisplay* display, IInputH* input, IAppNavigator* appNavigator);

    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    // Injected subsystems (non-owning).
    IDisplay*   display;
    IInputH*    input;
    IAppNavigator* appManager;

    // Sentinel: all bits set forces a full redraw on the first update(),
    // because every bit is treated as "changed" against this value.
    static constexpr uint16_t ALL_INPUTS_DIRTY = 0xFFFFu;

    // Cached debounced-state bitmap of the previous frame.
    uint16_t lastState = ALL_INPUTS_DIRTY;

    void drawArcadeBtn(int x, int y, int r, uint16_t color, bool pressed);
    void drawJoystick(int baseX, int baseY, bool up, bool down, bool left, bool right);
};
