/**
 * Project: Arcade Controller V1.3
 * File: InputReader.h
 * Description: Concrete IInputReader implementation. Owns the MCP23017
 *              I/O expander, initialises the ESP-side joystick GPIOs and
 *              folds both into a single ControlEvent-indexed bitmap on
 *              every readRaw() call. This is the hardware boundary of
 *              the input signal path — nothing above this class touches
 *              GPIO or I2C directly.
 */

#pragma once
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

#include "../config/Config.h"
#include "interfaces/IInputReader.h"

class InputReader : public IInputReader {
public:
    InputReader() = default;

    void begin() override;
    uint16_t readRaw() override;

private:
    Adafruit_MCP23X17 mcp;
    bool mcpConnected = false;
};
