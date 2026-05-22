/**
 * Project: Arcade Controller V1.3
 * File: IInputReader.h
 * Description: Abstract interface for the raw input subsystem. An
 *              IInputReader knows the hardware layout (which pin lives on
 *              which expander, which is a direct ESP GPIO) and produces a
 *              16-bit bitmap of currently pressed inputs. Debouncing and
 *              event dispatch happen one layer up, in InputHandler.
 *
 *              Bit position N in the returned bitmap mirrors the numeric
 *              value of ControlEvent N — the same encoding the rest of
 *              the project uses (getDebouncedStates(), HID_BUTTONS, ...).
 */

#pragma once
#include <Arduino.h>

class IInputReader {
public:
    virtual ~IInputReader() = default;

    // Brings the underlying hardware online (I2C bus, MCP23017, ESP pin modes).
    virtual void begin() = 0;

    // Returns the current raw input bitmap. Bit N = 1 iff the input mapped
    // to ControlEvent N is electrically pressed RIGHT NOW. No debouncing.
    virtual uint16_t readRaw() = 0;
};
