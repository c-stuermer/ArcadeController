/**
 * Project: Arcade Controller V1.3
 * File: BLEGamepadAdapter.cpp
 * Description: BLE-HID concrete transport. The shared dispatch logic
 *              (ControlEvent -> HID lookup, joystick state, hat
 *              direction) lives in GamepadOutput; this file only wires
 *              the backend hooks to the BleGamepad/NimBLE libraries.
 */

#include "BLEGamepadAdapter.h"

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

// --- Backend hooks (called from GamepadOutput::press/release) ------------

void BleGamepadAdapter::doPressButton(uint8_t hidIndex) {
    ble->press(hidIndex);
    ble->sendReport();
}

void BleGamepadAdapter::doReleaseButton(uint8_t hidIndex) {
    ble->release(hidIndex);
    ble->sendReport();
}

void BleGamepadAdapter::doSendJoystickReport() {
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

bool BleGamepadAdapter::isConnected()   { return ble->isConnected(); }
bool BleGamepadAdapter::isAdvertising() { return advertising; }

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
