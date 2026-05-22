/**
 * Project: Arcade Controller V1.3
 * File: transport/GamepadOutput.cpp
 * Description: Shared dispatch logic for every gamepad transport.
 */

#include "GamepadOutput.h"

namespace {

// Single source of truth for the ControlEvent -> HID button index mapping.
// Both press() and release() consult this table, so adding a new arcade
// button means adding exactly one row here.
struct HidMapping {
    ControlEvent ev;
    uint8_t      hidButton;
};

// HID button indices are 1..N (per the USB HID spec). Concrete adapters
// translate these to whatever their backend library expects.
constexpr HidMapping HID_BUTTONS[] = {
    { ControlEvent::BTN_A,      1  },
    { ControlEvent::BTN_B,      2  },
    { ControlEvent::BTN_X,      3  },
    { ControlEvent::BTN_Y,      4  },
    { ControlEvent::BTN_L1,     5  },
    { ControlEvent::BTN_R1,     6  },
    { ControlEvent::BTN_L2,     7  },
    { ControlEvent::BTN_R2,     8  },
    { ControlEvent::BTN_SELECT, 9  },
    { ControlEvent::BTN_START,  10 },
};

} // namespace

// --- Public dispatch -------------------------------------------------------

void GamepadOutput::press(ControlEvent event) {
    if (!isConnected()) return;

    // Joystick events update internal state + hat report directly;
    // non-joystick events fall through to the button mapping.
    if (handleJoystick(event, /*pressed=*/true)) return;

    if (uint8_t hid = hidButtonFor(event)) {
        doPressButton(hid);
    }
}

void GamepadOutput::release(ControlEvent event) {
    if (!isConnected()) return;

    if (handleJoystick(event, /*pressed=*/false)) return;

    if (uint8_t hid = hidButtonFor(event)) {
        doReleaseButton(hid);
    }
}

// --- Internal helpers ------------------------------------------------------

bool GamepadOutput::handleJoystick(ControlEvent event, bool pressed) {
    switch (event) {
        case ControlEvent::JOY_UP:    joyUp    = pressed; break;
        case ControlEvent::JOY_DOWN:  joyDown  = pressed; break;
        case ControlEvent::JOY_LEFT:  joyLeft  = pressed; break;
        case ControlEvent::JOY_RIGHT: joyRight = pressed; break;
        default: return false;  // not a joystick event
    }
    doSendJoystickReport();
    return true;
}

uint8_t GamepadOutput::hidButtonFor(ControlEvent ev) {
    for (const auto& m : HID_BUTTONS) {
        if (m.ev == ev) return m.hidButton;
    }
    return 0;
}
