/**
 * Project: Arcade Controller V1.3
 * File: BLEGamepadAdapter.h
 * Description: BLE-HID concrete transport. Inherits from the GamepadOutput
 *              abstract base class which owns the shared dispatch logic
 *              (HID button lookup, joystick state, hat-direction).
 *              This subclass only implements the small set of backend
 *              hooks (doPressButton / doReleaseButton / doSendJoystickReport)
 *              plus the connection lifecycle.
 */

#pragma once
#include "GamepadOutput.h"
#include <BleGamepad.h>     // Handles HID reports to emulate a standard game controller
#include <NimBLEDevice.h>   // Optimized, low-memory Bluetooth stack for ESP32

class BleGamepadAdapter : public GamepadOutput {
public:
    BleGamepadAdapter();
    // No custom destructor: 'ble' is intentionally never deleted -- the
    // adapter lives for the entire program lifetime on this device.

    // --- GamepadOutput backend lifecycle / state ---
    void begin() override;
    bool isConnected() override;
    bool isAdvertising() override;
    void disconnect() override;
    void startAdvertising() override;
    void stopAdvertising() override;
    void setBatteryLevel(int level) override;

protected:
    // --- GamepadOutput backend hooks ---
    void doPressButton(uint8_t hidIndex)   override;
    void doReleaseButton(uint8_t hidIndex) override;
    void doSendJoystickReport()            override;

private:
    BleGamepad*             ble;
    BleGamepadConfiguration config;

    // Tracked locally because the BLE stack does not expose its
    // advertising state in a portable way.
    bool advertising = false;
};
