/**
 * Project: Arcade Controller V1.1
 * File: IGamepadOutput.h
 * Description: Interface for gamepad drivers. BLE-HID implemented;
 *              a Switch 2 profile is planned (see Roadmap).
 */

#pragma once
#include "config/Config.h"

class IGamepadOutput {
public:
    // Virtual destructor is best practice for interfaces
    virtual ~IGamepadOutput() {} 

    virtual void begin() = 0;
    virtual void press(ControlEvent event) = 0;
    virtual void release(ControlEvent event) = 0;
    
    virtual bool isConnected() = 0;
    virtual bool isAdvertising() = 0;
    
    virtual void disconnect() = 0;
    virtual void startAdvertising() = 0;
    virtual void stopAdvertising() = 0;
    
    // Updates the battery level sent to the host device
    virtual void setBatteryLevel(int level) = 0;
};