/**
 * Project: Arcade Controller V0.1
 * File: Config.h
 * Description: Global hardware pinout and event definitions.
 */

#pragma once

//////////////////////////
// HARDWARE CONFIGURATION
//////////////////////////

// Define where a button is connected
enum class PinType {
    MCP, // Connected to MCP23017 I/O Expander
    ESP  // Connected directly to ESP32 GPIO
};

// Struct to group pin number and type
struct HardwarePin {
    int pin;
    PinType type;
};

namespace PinConfig {

    constexpr int MCP_ADDRESS = 0x20;
    constexpr int DEFAULT_DEBOUNCE_MS = 20;
    
    // --- MCP23017 Button Mapping ---
    constexpr HardwarePin ARCADE_A      = { 0, PinType::MCP };
    constexpr HardwarePin ARCADE_B      = { 1, PinType::MCP };
    constexpr HardwarePin ARCADE_X      = { 2, PinType::MCP };
    constexpr HardwarePin ARCADE_Y      = { 3, PinType::MCP };
    constexpr HardwarePin ARCADE_L1     = { 6, PinType::MCP };
    constexpr HardwarePin ARCADE_R1     = { 5, PinType::MCP };
    constexpr HardwarePin ARCADE_L2     = { 4, PinType::MCP };
    constexpr HardwarePin ARCADE_R2     = { 7, PinType::MCP };
    constexpr HardwarePin ARCADE_SELECT = { 8, PinType::MCP };
    constexpr HardwarePin ARCADE_START  = { 9, PinType::MCP };

    // --- ESP32 Joystick Mapping ---
    constexpr HardwarePin JOYSTICK_UP    = {  4, PinType::ESP };
    constexpr HardwarePin JOYSTICK_DOWN  = { 16, PinType::ESP };
    constexpr HardwarePin JOYSTICK_LEFT  = { 17, PinType::ESP };
    constexpr HardwarePin JOYSTICK_RIGHT = { 14, PinType::ESP };

    // --- System Controls ---
    constexpr HardwarePin POWER          = { 15, PinType::ESP };
    // Note: RESET is handled via hardware RST pin

    // --- Status & Sensors ---
    constexpr HardwarePin SYSTEM_LED     = { 13, PinType::ESP };
    constexpr int         BATTERY_AD     = 34; // FireBeetle internal voltage divider

    // --- I2C Bus ---
    constexpr int         I2C_VCC        = 26; // Power control for I2C peripherals
    constexpr int         I2C_SDA        = 21;
    constexpr int         I2C_SCL        = 22;
}

///////////////////////
// EVENT CONFIGURATION
///////////////////////

enum class EventType { 
    PRESSED, 
    RELEASED 
};

enum class ControlEvent {
    BTN_A, 
    BTN_B, 
    BTN_X, 
    BTN_Y,
    BTN_L1, 
    BTN_R1, 
    BTN_L2, 
    BTN_R2,
    BTN_SELECT, 
    BTN_START,
    JOY_UP, 
    JOY_DOWN, 
    JOY_LEFT, 
    JOY_RIGHT,
    SYS_RESET,
    SYS_POWER,
    NONE
};

// Helper: Convert ControlEvent to String (useful for debugging)
inline const char* eventToString(ControlEvent ev) {
    switch(ev) {
        case ControlEvent::BTN_A: return "BTN_A";
        case ControlEvent::BTN_B: return "BTN_B";
        case ControlEvent::BTN_X: return "BTN_X";
        case ControlEvent::BTN_Y: return "BTN_Y";
        case ControlEvent::BTN_L1: return "BTN_L1";
        case ControlEvent::BTN_R1: return "BTN_R1";
        case ControlEvent::BTN_L2: return "BTN_L2";
        case ControlEvent::BTN_R2: return "BTN_R2";
        case ControlEvent::BTN_SELECT: return "BTN_SELECT";
        case ControlEvent::BTN_START: return "BTN_START";
        case ControlEvent::JOY_UP: return "JOY_UP";
        case ControlEvent::JOY_DOWN: return "JOY_DOWN";
        case ControlEvent::JOY_LEFT: return "JOY_LEFT";
        case ControlEvent::JOY_RIGHT: return "JOY_RIGHT";
        case ControlEvent::SYS_RESET: return "SYS_RESET";
        case ControlEvent::SYS_POWER: return "SYS_POWER";
        default: return "UNKNOWN";
    }
}