/**
 * Project: Arcade Controller V1.3
 * File: apps/BluetoothApp/BluetoothApp.h
 * Description: Owns the gamepad transport (BLE-HID; Switch 2 planned) and
 *              provides the local Bluetooth UI / state machine:
 *              IDLE -> PAIRING -> CONNECTED -> PAUSED.
 *
 *              V1.3: interface-injected, no ISystem dependency. Holds an
 *              abstract GamepadOutput* (active transport) — the Switch 2
 *              adapter will plug in here once implemented.
 */

#pragma once
#include "../App.h"
#include "../AppId.h"
#include "../Interfaces/IAppNavigator.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/IPower.h"
#include "../../hal/interfaces/ISetting.h"
#include "../../hal/interfaces/IInputH.h"
#include "../../transport/GamepadOutput.h"
#include "../../transport/BLEGamepadAdapter.h"


// UI / control flow states for the BluetoothApp screen
enum class BtState { IDLE, PAIRING, CONNECTED, PAUSED };

class BluetoothApp : public App {
public:
    enum class Mode : uint8_t {
        BLE_HID = 0,  // Standard BLE HID gamepad (PC, phone, RPi, ...)
        SWITCH  = 1   // Nintendo Switch HID profile
    };

    BluetoothApp(IDisplay*   display,
                 IPower*     power,
                 ISetting*   settings,
                 IInputH*    input,
                 IAppNavigator* appNavigator);

    // --- App lifecycle ---
    void start()  override;
    void update() override;
    void stop()   override;
    void onInput(ControlEvent ev, EventType type) override;

    // --- Mode management ---
    void switchMode(Mode newMode);
    Mode getMode() const { return currentMode; }

private:
    // Injected subsystems (non-owning)
    IDisplay*   display;
    IPower*     power;
    ISetting*   settings;
    IInputH*    input;
    IAppNavigator* appNavigator;

    // Both adapters live as members. Only one is active at a time.
    BleGamepadAdapter      bleAdapter;
    // Switch2GamepadAdapter  switchAdapter;  // enable once implemented
    GamepadOutput*         active = nullptr;
    Mode                   currentMode = Mode::BLE_HID;
    bool                   adapterInitialized = false;

    // UI state machine
    BtState currentState   = BtState::IDLE;
    BtState lastDrawnState = BtState::PAUSED;  // forces initial draw

    // Battery push cadence to the gamepad host
    unsigned long lastBatterySync = 0;
    static constexpr unsigned long BATTERY_SYNC_MS = 60000;

    // Internal helpers
    GamepadOutput* adapterFor(Mode mode);
    void applyMode(Mode mode, bool persist);
    void drawScreen();
};
