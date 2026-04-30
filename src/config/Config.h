/**
 * Project: Arcade Controller V0.2 (SPI Upgrade)
 * File: Config.h
 * Description: Global hardware pinout and event definitions.
 */

#pragma once

//////////////////////////
// HARDWARE CONFIGURATION
//////////////////////////

// Defines where a button is physically connected
enum class PinType {
    MCP, // Connected via MCP23017 I/O Expander (I2C)
    ESP  // Connected directly to ESP32 GPIO
};

// Groups pin number and connection type
struct HardwarePin {
    int pin;
    PinType type;
};

namespace PinConfig {

    // --- Bus Settings ---
    constexpr int MCP_ADDRESS = 0x20;
    constexpr int DEFAULT_DEBOUNCE_MS = 15;
    
    // --- MCP23017 Button Mapping ---
    // Note: These use internal pull-ups (LOW = Pressed)
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
    
    // --- Joystick Mapping (Direct ESP GPIOs) ---
    // Note: These use internal pull-ups (LOW = Pressed)
    constexpr HardwarePin JOYSTICK_UP    = { 12, PinType::ESP };
    constexpr HardwarePin JOYSTICK_DOWN  = { 13, PinType::ESP };
    constexpr HardwarePin JOYSTICK_LEFT  = { 14, PinType::ESP };
    constexpr HardwarePin JOYSTICK_RIGHT = { 27, PinType::ESP };
    
    // --- System Pins ---
    constexpr HardwarePin SYSTEM_LED = { 2,  PinType::ESP }; // Built-in LED
    constexpr HardwarePin BATTERY_AD = { 34, PinType::ESP }; // Analog Battery Monitor
    constexpr HardwarePin POWER      = { 35, PinType::ESP }; // Physical Power Switch

    // --- Sound Settings ---
    constexpr int SOUND_PWM_PIN = 25; // DAC Pin für Audio
    constexpr int SOUND_CHANNEL = 1;  // LEDC PWM Kanal
    
    // --- Communication Bus (I2C) ---
    constexpr int I2C_SDA = 21;
    constexpr int I2C_SCL = 22;

    // --- Communication Bus (SPI Display) ---
    constexpr int SPI_MOSI = 23;  // Master Out Slave In: Transmits pixel and command data from ESP32 to display
    constexpr int SPI_SCLK = 18;  // Serial Clock: Synchronizes the data transfer speed
    constexpr int DISP_CS  = 5;   // Chip Select: Activates the display on the SPI bus (useful if multiple SPI devices share the bus)
    constexpr int DISP_DC  = 17;  // Data/Command: Tells the display if incoming bytes are structural commands (LOW) or actual color pixels (HIGH)
    constexpr int DISP_RST = 4;   // Reset Pin: Hardware reset to re-initialize the screen
    constexpr int DISP_BLK = 16;  // Backlight PWM: Controls screen brightness 
}

/////////////////////////
// EVENT CONFIGURATION //
/////////////////////////

enum class EventType { 
    PRESSED, 
    RELEASED 
};

//Defines all logical actions that can be triggered by the hardware.
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

// Helper: Converts a ControlEvent enum to a string for logging/UI purposes.
inline const char* eventToString(ControlEvent ev) {
    switch(ev) {
        case ControlEvent::BTN_A:      return "BTN_A";
        case ControlEvent::BTN_B:      return "BTN_B";
        case ControlEvent::BTN_X:      return "BTN_X";
        case ControlEvent::BTN_Y:      return "BTN_Y";
        case ControlEvent::BTN_L1:     return "BTN_L1";
        case ControlEvent::BTN_R1:     return "BTN_R1";
        case ControlEvent::BTN_L2:     return "BTN_L2";
        case ControlEvent::BTN_R2:     return "BTN_R2";
        case ControlEvent::BTN_SELECT: return "BTN_SELECT";
        case ControlEvent::BTN_START:  return "BTN_START";
        case ControlEvent::JOY_UP:     return "JOY_UP";
        case ControlEvent::JOY_DOWN:   return "JOY_DOWN";
        case ControlEvent::JOY_LEFT:   return "JOY_LEFT";
        case ControlEvent::JOY_RIGHT:  return "JOY_RIGHT";
        case ControlEvent::SYS_RESET:  return "SYS_RESET";
        case ControlEvent::SYS_POWER:  return "SYS_POWER";
        default:                       return "NONE";
    }
}