/**
 * Project: Arcade Controller V0.1
 * File: InputHandler.h
 * Description: Manages inputs from direct GPIOs and I2C Expander (MCP23017).
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

// IMPORTANT: Config must be included first for ControlEvent enums
#include "../config/Config.h" 
#include "../driver/Button.h"
#include "../transport/IGamepadOutput.h"

struct ArcadeButtonDef {
    HardwarePin hw;
    Button logic;
    ControlEvent eventId;
};

class InputHandler {
public:
    // Define callback type (ControlEvent, PRESSED/RELEASED)
    using EventCallback = std::function<void(ControlEvent, EventType)>;

    InputHandler();
    void init();
    void update(IGamepadOutput* gamepad);
    
    void onEvent(EventCallback cb) { _callback = cb; }
    
    // State Checks
    bool isPressed(ControlEvent ev); 
    bool isPressed(HardwarePin pinConfig);

    // Duration Checks (e.g., for holding buttons)
    unsigned long getDuration(ControlEvent ev);
    unsigned long getLastPressDuration(ControlEvent ev);

private:
    Adafruit_MCP23X17 mcp;
    bool mcpConnected = false;
    uint16_t mcpState = 0; 

    std::vector<ArcadeButtonDef> buttons;
    unsigned long lastHardwareRead = 0;
    
    EventCallback _callback = nullptr; 
};