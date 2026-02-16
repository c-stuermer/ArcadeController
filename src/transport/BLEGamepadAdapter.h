/**
 * Project: Arcade Controller V0.1
 * File: BleGamepadAdapter.h
 * Description: Wrapper for the ESP32-BLE-Gamepad library.
 */

#pragma once
#include "IGamepadOutput.h"
#include <BleGamepad.h>

class BleGamepadAdapter : public IGamepadOutput {
private:
    BleGamepad* ble;
    BleGamepadConfiguration config; 

public:
    BleGamepadAdapter() {
        // Initialize the library object
        // Arguments: Name, Manufacturer, Battery Level
        ble = new BleGamepad("Arcade Controller", "DIY-Project", 100);
        
        // Configure HID Report
        config.setAutoReport(true); 
        config.setControllerType(CONTROLLER_TYPE_GAMEPAD); 
    }

    void begin() override {
        // Apply config and start BLE advertising
        ble->begin(&config); 
    }
    
    void press(ControlEvent event) override {
        // Mapping ControlEvent to BleGamepad specific buttons would happen here
        // For V0.1 we assume direct mapping or this is handled in InputHandler
        // TODO: Implement mapping based on 'event'
    }

    void release(ControlEvent event) override {
        // TODO: Implement mapping
    }

    bool isConnected() override {
        return ble->isConnected();
    }
};