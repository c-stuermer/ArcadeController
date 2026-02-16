/**
 * Project: Arcade Controller V0.1
 * File: IGamepadOutput.h
 * Description: Interface for gamepad drivers (Real BLE vs. Dummy/Simulation).
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
};