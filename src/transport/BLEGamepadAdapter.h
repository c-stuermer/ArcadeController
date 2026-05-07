/**
 * Project: Arcade Controller V0.2
 * File: BLEGamepadAdapter.h
 * Description: Implementation of the gamepad interface using ESP32 BLE.
 */

#pragma once
#include "IGamepadOutput.h"
#include <BleGamepad.h>         // Handles HID reports to emulate a standard game controller
#include <NimBLEDevice.h>       // Optimized, low-memory Bluetooth stack for ESP32

class BleGamepadAdapter : public IGamepadOutput {
private:
    BleGamepad* ble;
    BleGamepadConfiguration config; 

    // Stores the current state of the joystick axes
    bool joyUp = false;
    bool joyDown = false;
    bool joyLeft = false;
    bool joyRight = false;

    // Helper function to calculate diagonal movement (8-way D-Pad)
    void updateHat() {
        if (joyUp && joyRight)          ble->setHat1(HAT_UP_RIGHT);
        else if (joyUp && joyLeft)      ble->setHat1(HAT_UP_LEFT);
        else if (joyDown && joyRight)   ble->setHat1(HAT_DOWN_RIGHT);
        else if (joyDown && joyLeft)    ble->setHat1(HAT_DOWN_LEFT);
        else if (joyUp)                 ble->setHat1(HAT_UP);
        else if (joyDown)               ble->setHat1(HAT_DOWN);
        else if (joyLeft)               ble->setHat1(HAT_LEFT);
        else if (joyRight)              ble->setHat1(HAT_RIGHT);
        else                            ble->setHat1(HAT_CENTERED);
        
        ble->sendReport();
    }

public:
    BleGamepadAdapter() {
        ble = new BleGamepad("Arcade Controller", "IDDQD", 50);  // TODO: find bug in battery load transmission

        //HID-Report-Descriptort
        config.setAutoReport(false); 
        config.setControllerType(CONTROLLER_TYPE_GAMEPAD); 
        config.setHatSwitchCount(1);
        config.setButtonCount(10); 
    }

    void begin() override {
        ble->begin(&config); 
    }
    
    void press(ControlEvent event) override {
        if(!ble->isConnected()) return;

        // Update joystick state
        if (event == ControlEvent::JOY_UP)    { joyUp = true; updateHat(); return; }
        if (event == ControlEvent::JOY_DOWN)  { joyDown = true; updateHat(); return; }
        if (event == ControlEvent::JOY_LEFT)  { joyLeft = true; updateHat(); return; }
        if (event == ControlEvent::JOY_RIGHT) { joyRight = true; updateHat(); return; }

        // Normal buttons
        switch(event) {
            case ControlEvent::BTN_A:      ble->press(BUTTON_1); break;
            case ControlEvent::BTN_B:      ble->press(BUTTON_2); break;
            case ControlEvent::BTN_X:      ble->press(BUTTON_3); break;
            case ControlEvent::BTN_Y:      ble->press(BUTTON_4); break;
            case ControlEvent::BTN_L1:     ble->press(BUTTON_5); break;
            case ControlEvent::BTN_R1:     ble->press(BUTTON_6); break;
            case ControlEvent::BTN_L2:     ble->press(BUTTON_7); break;
            case ControlEvent::BTN_R2:     ble->press(BUTTON_8); break;
            case ControlEvent::BTN_SELECT: ble->press(BUTTON_9); break;
            case ControlEvent::BTN_START:  ble->press(BUTTON_10); break;
            default: break;
        }
        ble->sendReport();
    }

    void release(ControlEvent event) override {
        if(!ble->isConnected()) return;

        // Update joystick state
        if (event == ControlEvent::JOY_UP)    { joyUp = false; updateHat(); return; }
        if (event == ControlEvent::JOY_DOWN)  { joyDown = false; updateHat(); return; }
        if (event == ControlEvent::JOY_LEFT)  { joyLeft = false; updateHat(); return; }
        if (event == ControlEvent::JOY_RIGHT) { joyRight = false; updateHat(); return; }

        // Normal buttons
        switch(event) {
            case ControlEvent::BTN_A:      ble->release(BUTTON_1); break;
            case ControlEvent::BTN_B:      ble->release(BUTTON_2); break;
            case ControlEvent::BTN_X:      ble->release(BUTTON_3); break;
            case ControlEvent::BTN_Y:      ble->release(BUTTON_4); break;
            case ControlEvent::BTN_L1:     ble->release(BUTTON_5); break;
            case ControlEvent::BTN_R1:     ble->release(BUTTON_6); break;
            case ControlEvent::BTN_L2:     ble->release(BUTTON_7); break;
            case ControlEvent::BTN_R2:     ble->release(BUTTON_8); break;
            case ControlEvent::BTN_SELECT: ble->release(BUTTON_9); break;
            case ControlEvent::BTN_START:  ble->release(BUTTON_10); break;
            default: break;
        }
        ble->sendReport();
    }

    void disconnect() override {
        // Correct way to disconnect all peers using NimBLE
        if (ble->isConnected()) {
            // 0xFFFF is a special constant to disconnect all
            NimBLEDevice::getServer()->disconnect(0xFFFF); 
        }
    }

    bool isConnected() override { return ble->isConnected(); }
    bool isAdvertising() override { return !ble->isConnected(); }
    
    void startAdvertising() override {
        NimBLEDevice::getAdvertising()->start();
    }
    
    void stopAdvertising() override {
        NimBLEDevice::getAdvertising()->stop();
    }
    
    void setBatteryLevel(int level) override {
        ble->setBatteryLevel(level);                // TODO: find bug in battery load transmission
    }
};