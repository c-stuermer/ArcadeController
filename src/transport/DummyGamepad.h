/**
 * Project: Arcade Controller V0.1
 * File: DummyGamepad.h
 * Description: Mock driver for debugging without Bluetooth stack overhead.
 */

#pragma once
#include <Arduino.h>
#include "IGamepadOutput.h"

class DummyGamepad : public IGamepadOutput {
public:
    void begin() override {
        Serial.println("[MOCK] Gamepad Service started (Simulation Mode).");
    }

    void press(ControlEvent event) override {
        // Debug output to see what WOULD be sent
        // Serial.printf("[MOCK] Press Event ID: %d\n", (int)event);
    }

    void release(ControlEvent event) override {
        // Serial.printf("[MOCK] Release Event ID: %d\n", (int)event);
    }

    bool isConnected() override {
        // Always return true to allow input logic testing
        return true; 
    }
};