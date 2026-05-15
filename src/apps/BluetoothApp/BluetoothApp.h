/**
 * Project: Arcade Controller V1.1
 * File: apps/BluetoothApp/BluetoothApp.h
 * Description: Owns the gamepad transport (BLE-HID; Switch 2 planned) and
 *              provides the local Bluetooth UI / state machine that the
 *              V0.2 build had: IDLE -> PAIRING -> CONNECTED -> PAUSED.
 */

#pragma once
#include "../App.h"
#include "../../transport/IGamepadOutput.h"
#include "../../transport/BLEGamepadAdapter.h"
// #include "../../transport/Switch2GamepadAdapter.h"  // enable once implemented

// UI / control flow states for the BluetoothApp screen
enum class BtState { IDLE, PAIRING, CONNECTED, PAUSED };

class BluetoothApp : public App {
public:
    enum class Mode : uint8_t {
        BLE_HID = 0,  // Standard BLE HID gamepad (PC, phone, RPi, ...)
        SWITCH  = 1   // Nintendo Switch HID profile
    };

    explicit BluetoothApp(ISystem* sys);

    // --- App lifecycle ---
    void start()  override;
    void update() override;
    void stop()   override;
    void onInput(ControlEvent ev, EventType type) override;

    // --- Mode management ---
    // Switches the active transport at runtime and persists the choice.
    void switchMode(Mode newMode);
    Mode getMode() const { return currentMode; }

private:
    // Both adapters live as members. Only one is active at a time.
    BleGamepadAdapter      bleAdapter;
    // Switch2GamepadAdapter  switchAdapter;  // enable once implemented
    IGamepadOutput*        active = nullptr;
    Mode                   currentMode = Mode::BLE_HID;
    bool                   adapterInitialized = false;

    // UI state machine (V0.2 behaviour)
    BtState currentState   = BtState::IDLE;
    BtState lastDrawnState = BtState::PAUSED;  // forces initial draw

    // Battery is pulled from PowerManager periodically (replaces V0.2's
    // push-style syncSystemStats() in ArcadeController).
    unsigned long lastBatterySync = 0;
    static constexpr unsigned long BATTERY_SYNC_MS = 60000;

    // Internal helpers
    IGamepadOutput* adapterFor(Mode mode);
    void applyMode(Mode mode, bool persist);
    void drawScreen();
};
