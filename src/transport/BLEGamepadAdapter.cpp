/**
 * Project: Arcade Controller V1.2
 * File: BLEGamepadAdapter.cpp
 * Description: BLE-HID gamepad transport. Maps logical ControlEvents to
 *              HID button indices via a single static table (HID_BUTTONS).
 */

#include "BLEGamepadAdapter.h"

namespace {

// --- HID button mapping ---------------------------------------------------
// Single source of truth for the ControlEvent -> HID button index mapping.
// Both press() and release() consult this table, so adding a new arcade
// button means adding exactly one row here.
struct HidMapping {
    ControlEvent ev;
    uint8_t      hidButton;
};

constexpr HidMapping HID_BUTTONS[] = {
    { ControlEvent::BTN_A,      BUTTON_1  },
    { ControlEvent::BTN_B,      BUTTON_2  },
    { ControlEvent::BTN_X,      BUTTON_3  },
    { ControlEvent::BTN_Y,      BUTTON_4  },
    { ControlEvent::BTN_L1,     BUTTON_5  },
    { ControlEvent::BTN_R1,     BUTTON_6  },
    { ControlEvent::BTN_L2,     BUTTON_7  },
    { ControlEvent::BTN_R2,     BUTTON_8  },
    { ControlEvent::BTN_SELECT, BUTTON_9  },
    { ControlEvent::BTN_START,  BUTTON_10 },
};

// Look up the HID button index for a given logical event.
// Returns 0 if the event has no mapping (e.g. joystick events).
uint8_t hidButtonFor(ControlEvent ev) {
    for (const auto& m : HID_BUTTONS) {
        if (m.ev == ev) return m.hidButton;
    }
    return 0;
}

} // namespace

// --- Construction ---------------------------------------------------------

BleGamepadAdapter::BleGamepadAdapter() {
    ble = new BleGamepad("Arcade Controller", "IDDQD", 50);

    // HID report descriptor
    config.setAutoReport(false);
    config.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    config.setHatSwitchCount(1);
    config.setButtonCount(10);
}

void BleGamepadAdapter::begin() {
    ble->begin(&config);
}

// --- Input -> HID --------------------------------------------------------

void BleGamepadAdapter::press(ControlEvent event) {
    if (!ble->isConnected()) return;

    // Joystick events update the hat directly; non-joy events fall through.
    if (handleJoystick(event, /*pressed=*/true)) return;

    if (uint8_t hid = hidButtonFor(event)) {
        ble->press(hid);
        ble->sendReport();
    }
}

void BleGamepadAdapter::release(ControlEvent event) {
    if (!ble->isConnected()) return;

    if (handleJoystick(event, /*pressed=*/false)) return;

    if (uint8_t hid = hidButtonFor(event)) {
        ble->release(hid);
        ble->sendReport();
    }
}

bool BleGamepadAdapter::handleJoystick(ControlEvent event, bool pressed) {
    switch (event) {
        case ControlEvent::JOY_UP:    joyUp    = pressed; break;
        case ControlEvent::JOY_DOWN:  joyDown  = pressed; break;
        case ControlEvent::JOY_LEFT:  joyLeft  = pressed; break;
        case ControlEvent::JOY_RIGHT: joyRight = pressed; break;
        default: return false;  // not a joystick event
    }
    updateHat();
    return true;
}

void BleGamepadAdapter::updateHat() {
    // 8-way D-Pad: diagonals first, then cardinals, then center.
    if      (joyUp   && joyRight)   ble->setHat1(HAT_UP_RIGHT);
    else if (joyUp   && joyLeft)    ble->setHat1(HAT_UP_LEFT);
    else if (joyDown && joyRight)   ble->setHat1(HAT_DOWN_RIGHT);
    else if (joyDown && joyLeft)    ble->setHat1(HAT_DOWN_LEFT);
    else if (joyUp)                 ble->setHat1(HAT_UP);
    else if (joyDown)               ble->setHat1(HAT_DOWN);
    else if (joyLeft)               ble->setHat1(HAT_LEFT);
    else if (joyRight)              ble->setHat1(HAT_RIGHT);
    else                            ble->setHat1(HAT_CENTERED);

    ble->sendReport();
}

// --- Connection management -----------------------------------------------

void BleGamepadAdapter::disconnect() {
    if (ble->isConnected()) {
        // 0xFFFF is the NimBLE-specific "disconnect all peers" handle.
        NimBLEDevice::getServer()->disconnect(0xFFFF);
    }
}

bool BleGamepadAdapter::isConnected() {
    return ble->isConnected();
}

bool BleGamepadAdapter::isAdvertising() {
    return advertising;
}

void BleGamepadAdapter::startAdvertising() {
    NimBLEDevice::getAdvertising()->start();
    advertising = true;
}

void BleGamepadAdapter::stopAdvertising() {
    NimBLEDevice::getAdvertising()->stop();
    advertising = false;
}

void BleGamepadAdapter::setBatteryLevel(int level) {
    ble->setBatteryLevel(level);
}
