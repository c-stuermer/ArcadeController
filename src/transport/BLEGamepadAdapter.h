/**
 * Project: Arcade Controller V1.2
 * File: BLEGamepadAdapter.h
 * Description: Implementation of the gamepad interface using ESP32 BLE.
 *              Translates logical ControlEvents into HID button/hat reports
 *              via a single static mapping table (see .cpp).
 */

#pragma once
#include "IGamepadOutput.h"
#include <BleGamepad.h>     // Handles HID reports to emulate a standard game controller
#include <NimBLEDevice.h>   // Optimized, low-memory Bluetooth stack for ESP32

class BleGamepadAdapter : public IGamepadOutput {
public:
    BleGamepadAdapter();
    // No custom destructor: 'ble' is intentionally never deleted -- the
    // adapter lives for the entire program lifetime on this device, and
    // tearing down the NimBLE stack at shutdown is not needed.

    // --- IGamepadOutput ---
    void begin() override;
    void press(ControlEvent event) override;
    void release(ControlEvent event) override;

    bool isConnected() override;
    bool isAdvertising() override;

    void disconnect() override;
    void startAdvertising() override;
    void stopAdvertising() override;

    void setBatteryLevel(int level) override;

private:
    BleGamepad* ble;
    BleGamepadConfiguration config;

    // Joystick axis state. Combined into one hat-switch direction by updateHat().
    bool joyUp    = false;
    bool joyDown  = false;
    bool joyLeft  = false;
    bool joyRight = false;

    // Tracked locally because the BLE stack does not expose its
    // advertising state in a portable way.
    bool advertising = false;

    // Updates internal joystick state and re-sends the hat report.
    // Returns true if 'event' was a joystick direction (i.e. handled here),
    // false otherwise -> caller should fall through to the button mapping.
    bool handleJoystick(ControlEvent event, bool pressed);

    // Recomputes the 8-way hat direction from the four joystick bools and
    // sends a HID report.
    void updateHat();
};
