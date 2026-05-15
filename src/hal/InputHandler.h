/**
 * Project: Arcade Controller V1.1
 * File: InputHandler.h
 * Description: Manages inputs from direct GPIOs and the I2C Expander (MCP23017).
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

#include "../config/Config.h" 
#include "../driver/Button.h"

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
    
    // Initializes the I2C bus and configures pin modes
    void begin();
    
    // Polls the hardware states and triggers events. Must be called in the main loop.
    void update();
    
    // Registers the callback function for input events
    void onEvent(EventCallback cb) { _callback = cb; }
    
    // --- State Checks ---
    bool isPressed(ControlEvent ev); 
    bool isPressed(HardwarePin pinConfig);

    // --- Duration Checks (e.g., for holding buttons) ---
    unsigned long getDuration(ControlEvent ev);
    unsigned long getLastPressDuration(ControlEvent ev);

    // --- Bulk Snapshot ---
    // Debounced bitmap of all logical inputs. Bit position N corresponds to
    // the ControlEvent enum value N (e.g. bit 0 = BTN_A, bit 10 = JOY_UP).
    // Intended for poll-style consumers (e.g. InputMonitorApp) that want all
    // inputs in one register-level read instead of N individual lookups.
    uint16_t getDebouncedStates() const { return _debouncedStates; }

private:
    Adafruit_MCP23X17 mcp;
    bool mcpConnected = false;
    uint16_t mcpState = 0;
    uint16_t _debouncedStates = 0;

    std::vector<ArcadeButtonDef> buttons;
    unsigned long lastHardwareRead = 0;

    EventCallback _callback = nullptr;
};