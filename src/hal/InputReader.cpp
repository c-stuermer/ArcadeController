/**
 * Project: Arcade Controller V1.3
 * File: InputReader.cpp
 */

#include "InputReader.h"

void InputReader::begin() {
    Wire.begin();
    Wire.setClock(400000);   // Fast I2C clock

    // Probe the MCP23017
    Wire.beginTransmission(PinConfig::MCP_ADDRESS);
    if (Wire.endTransmission() == 0) {
        Serial.println("[I2C] MCP23017 connected!");
        mcpConnected = true;
        mcp.begin_I2C(PinConfig::MCP_ADDRESS);

        // Arcade buttons hang on the MCP with internal pull-ups.
        for (const auto& in : PinConfig::ARCADE_INPUTS) {
            mcp.pinMode(in.hw.pin, INPUT_PULLUP);
        }
    } else {
        Serial.println("[I2C] MCP23017 NOT found!");
        mcpConnected = false;
    }

    // Joystick microswitches live on direct ESP GPIOs.
    for (const auto& in : PinConfig::JOYSTICK_INPUTS) {
        pinMode(in.hw.pin, INPUT_PULLUP);
    }
}

uint16_t InputReader::readRaw() {
    // 1. Bulk-read the MCP once per tick. INPUT_PULLUP is LOW when
    //    pressed -> invert so '1' means 'pressed'.
    uint16_t mcpState = 0;
    if (mcpConnected) {
        mcpState = ~mcp.readGPIOAB();
    }

    // 2. Fold the two pin sources into one ControlEvent-indexed bitmap.
    //    Bit position == numeric value of ControlEvent.
    uint16_t raw = 0;

    for (const auto& in : PinConfig::ARCADE_INPUTS) {
        if (!mcpConnected) continue;
        const bool pressed = (mcpState >> in.hw.pin) & 1;
        if (pressed) raw |= (1u << static_cast<int>(in.event));
    }

    for (const auto& in : PinConfig::JOYSTICK_INPUTS) {
        const bool pressed = (digitalRead(in.hw.pin) == LOW);
        if (pressed) raw |= (1u << static_cast<int>(in.event));
    }

    return raw;
}
