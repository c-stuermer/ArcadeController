/**
 * Project: Arcade Controller V0.2
 * File: BluetoothApp.h
 * Description: Application logic for Bluetooth pairing and gamepad passthrough.
 * TODO: Implement onFocusLost() function - resets all presses when entering the menu 
 *       or quitting the Bluetooth app to prevent permanent inputs on the host.
 */

#pragma once
#include "../App.h" 
#include <TFT_eSPI.h>

// States to control the application flow
enum class BtState { IDLE, PAIRING, CONNECTED, PAUSED };

class BluetoothApp : public App {
public:
    using App::App; 

    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    BtState currentState = BtState::IDLE;
    BtState lastDrawnState = BtState::PAUSED; // Forces initial drawing

    void drawScreen();
};