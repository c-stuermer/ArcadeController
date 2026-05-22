/**
 * Project: Arcade Controller V1.3
 * File: InputMonitorApp.cpp
 * Description: Implementation of the input monitor UI and logic.
 */

#include "InputMonitorApp.h"
#include "../../config/Colors.h"

// Helper: bit position for a logical input within the debounced-state bitmap.
static constexpr uint16_t EVENT_BIT(ControlEvent e) {
    return 1u << static_cast<int>(e);
}

static constexpr uint16_t JOY_MASK =
      EVENT_BIT(ControlEvent::JOY_UP)   | EVENT_BIT(ControlEvent::JOY_DOWN)
    | EVENT_BIT(ControlEvent::JOY_LEFT) | EVENT_BIT(ControlEvent::JOY_RIGHT);

InputMonitorApp::InputMonitorApp(IDisplay* display, IInputH* input, IAppNavigator* appNavigator)
    : App(),
      display(display),
      input(input),
      appManager(appNavigator) {}

void InputMonitorApp::start() {
    Serial.println("[APP] InputMonitorApp starting...");
    display->clear();

    // Static text for exit combo
    display->drawText(10, 112, "[SELECT] + [L2] + [R2]", Colors::RED);

    lastState = ALL_INPUTS_DIRTY;   // Forces a full redraw on first update
}

void InputMonitorApp::stop() {
    // Ensure the progress bar is cleared when exiting the app
    display->clearProgressBar();
}

void InputMonitorApp::onInput(ControlEvent ev, EventType type) {
    // Inputs are handled directly via polling in update()
}

void InputMonitorApp::update() {
    display->drawHeader("INPUT MONITOR");

    // One-shot snapshot of all debounced inputs (maintained by InputHandler).
    uint16_t currentState = input->getDebouncedStates();

    // Pure calculation: no RAM access, only internal CPU registers and ALU
    uint16_t changed = currentState ^ lastState;

    if (changed != 0) {
        // Capture the HAL-side edge timestamp once, BEFORE drawing. The delta
        // at the end measures the full path: edge detected in InputHandler ->
        // bitmap published -> read here -> redraw finished.
        const unsigned long edgeMs = input->getLastEdgeMs();

        // Joystick (any direction changed -> redraw the whole stick)
        if (changed & JOY_MASK) {
            drawJoystick(35, 80,
                currentState & EVENT_BIT(ControlEvent::JOY_UP),
                currentState & EVENT_BIT(ControlEvent::JOY_DOWN),
                currentState & EVENT_BIT(ControlEvent::JOY_LEFT),
                currentState & EVENT_BIT(ControlEvent::JOY_RIGHT));
        }

        // Arcade Buttons (A, B, X, Y)
        if (changed & EVENT_BIT(ControlEvent::BTN_A)) drawArcadeBtn(75,  85, 9, Colors::RED,    currentState & EVENT_BIT(ControlEvent::BTN_A));
        if (changed & EVENT_BIT(ControlEvent::BTN_B)) drawArcadeBtn(97,  73, 9, Colors::YELLOW, currentState & EVENT_BIT(ControlEvent::BTN_B));
        if (changed & EVENT_BIT(ControlEvent::BTN_X)) drawArcadeBtn(121, 73, 9, Colors::GREEN,  currentState & EVENT_BIT(ControlEvent::BTN_X));
        if (changed & EVENT_BIT(ControlEvent::BTN_Y)) drawArcadeBtn(143, 85, 9, Colors::CYAN,   currentState & EVENT_BIT(ControlEvent::BTN_Y));

        // Shoulder Buttons (L2, L1, R1, R2)
        if (changed & EVENT_BIT(ControlEvent::BTN_L2)) drawArcadeBtn(75,  60, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_L2));
        if (changed & EVENT_BIT(ControlEvent::BTN_L1)) drawArcadeBtn(97,  48, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_L1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R1)) drawArcadeBtn(121, 48, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_R1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R2)) drawArcadeBtn(143, 60, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_R2));

        // Start & Select
        if (changed & EVENT_BIT(ControlEvent::BTN_START))  drawArcadeBtn(15, 40, 5, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_START));
        if (changed & EVENT_BIT(ControlEvent::BTN_SELECT)) drawArcadeBtn(30, 40, 5, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_SELECT));

        lastState = currentState;

        // Diagnostic line: real edge-to-screen latency (LAT) plus the
        // duration of the last completed press (DUR). DUR persists between
        // releases so it always shows the most recent hold time.
        String latText = "LAT: " + String(millis() - edgeMs)
                       + " ms  DUR: " + String(input->getLastReleasedDurationMs()) + " ms ";
        display->fillRect(2, 14, 130, 10, Colors::BLACK);
        display->drawText(2, 16, latText, Colors::GREEN);
    }

    // --- EXIT LOGIC: SELECT + L2 + R2 ---
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);

    // Combo logic: Only if all three are currently pressed
    if (durSel > 0 && durL2 > 0 && durR2 > 0) {
        // Shared hold time is the minimum duration among the three buttons
        unsigned long comboTime = min(durSel, min(durL2, durR2));

        display->drawProgressBar(comboTime, 2000, Colors::RED);

        // 2-Second Check
        if (comboTime >= 2000) {
            display->clearProgressBar();
            appManager->switchApp(AppId::Menu);
            return;
        }
    } else {
        // Clear the bar if any of the three buttons is released
        display->clearProgressBar();
    }
}

void InputMonitorApp::drawArcadeBtn(int x, int y, int r, uint16_t color, bool pressed) {
    // Filled body: pressed -> always white; otherwise the configured color
    // (BLACK system buttons stay black, others keep their hue).
    const uint16_t fill = pressed ? Colors::WHITE : color;
    display->fillCircle(x, y, r, fill);

    // Outline is always white.
    display->drawCircle(x, y, r, Colors::WHITE);
}

void InputMonitorApp::drawJoystick(int baseX, int baseY, bool up, bool down, bool left, bool right) {
    // Wipe the joystick area to avoid leftover knob pixels from the previous frame.
    const int half = 25;
    display->fillRect(baseX - half, baseY - half, 2*half, 2*half, Colors::BLACK);

    // Base ring
    display->drawCircle(baseX, baseY, 16, Colors::WHITE);

    // Knob position
    int kx = baseX;
    int ky = baseY;
    bool isDiagonal = (up || down) && (left || right);
    int step = isDiagonal ? 11 : 16;

    if (up)    ky -= step;
    if (down)  ky += step;
    if (left)  kx -= step;
    if (right) kx += step;

    bool pressed = (up || down || left || right);

    // Shaft (when deflected)
    if (pressed) {
        display->drawLine(baseX,     baseY, kx, ky, Colors::WHITE);
        display->drawLine(baseX - 1, baseY, kx, ky, Colors::WHITE);
    }

    // Knob
    display->fillCircle(kx, ky, 8, Colors::WHITE);
}
