/**
 * Project: Arcade Controller V1.1
 * File: Config.h
 * Description: Global hardware pinout and event definitions.
 *
 * Changes vs. V0.2:
 *  - Removed SYS_RESET / SYS_POWER from ControlEvent: the Power switch is
 *    owned by PowerManager (read directly, never becomes a ControlEvent),
 *    and Reset is handled by the ESP32's hardware EN-pin - no software
 *    involvement at all.
 */

#pragma once

//////////////////////////
// HARDWARE CONFIGURATION
//////////////////////////

enum class PinType {
    MCP, // Connected via MCP23017 I/O Expander (I2C)
    ESP  // Connected directly to ESP32 GPIO
};

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
    constexpr HardwarePin JOYSTICK_UP    = {  4, PinType::ESP };
    constexpr HardwarePin JOYSTICK_DOWN  = { 16, PinType::ESP };
    constexpr HardwarePin JOYSTICK_LEFT  = { 17, PinType::ESP };
    constexpr HardwarePin JOYSTICK_RIGHT = { 14, PinType::ESP };

    // --- System Pins ---
    constexpr HardwarePin SYSTEM_LED = { 13,  PinType::ESP }; // Built-in LED
    constexpr HardwarePin BATTERY_AD = { 34, PinType::ESP }; // Analog Battery Monitor
    constexpr HardwarePin POWER      = { 15, PinType::ESP }; // Physical Power Switch (toggle)
    // Reset is wired to the ESP32 EN-pin (hardware reset) - no GPIO needed.

    // --- Sound Settings ---
    constexpr int SOUND_PWM_PIN = 25;
    constexpr int SOUND_CHANNEL = 1;

    // --- Communication Bus (I2C) ---
    constexpr int I2C_SDA = 21;
    constexpr int I2C_SCL = 22;

    // --- Communication Bus (SPI Display) ---
    constexpr int SPI_SCLK = 18; // Terminal: SCK
    constexpr int SPI_MOSI = 23; // Terminal: MOSI
    constexpr int DISP_CS  = 0;  // Terminal: D5  (Chip Select)
    constexpr int DISP_DC  = 12; // Terminal: D13 (Data/Command)
    constexpr int DISP_RST = 19; // Terminal: MISO (Reset)
    constexpr int DISP_BLK = 26; // Terminal: D3  (Backlight, formerly I2C_VCC)
}

/////////////////////////
// EVENT CONFIGURATION //
/////////////////////////

enum class EventType {
    PRESSED,
    RELEASED
};

// Logical actions triggered by hardware. System buttons (Power, Reset) are
// intentionally absent: they never reach apps as ControlEvents.
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
    NONE
};

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
        default:                       return "NONE";
    }
}
