/**
 * Project: Arcade Controller V1.2
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
    void onEvent(EventCallback cb) { callback = cb; }

    // --- State Checks ---
    bool isPressed(ControlEvent ev);

    // --- Duration Checks (e.g., for holding buttons) ---
    unsigned long getDuration(ControlEvent ev);
    unsigned long getLastPressDuration(ControlEvent ev);

    // --- Bulk Snapshot ---
    // Debounced bitmap of all logical inputs. Bit position N corresponds to
    // the ControlEvent enum value N (e.g. bit 0 = BTN_A, bit 10 = JOY_UP).
    // Intended for poll-style consumers (e.g. InputMonitorApp) that want all
    // inputs in one register-level read instead of N individual lookups.
    uint16_t getDebouncedStates() const { return debouncedStates; }

    // --- Latency ---
    // millis() timestamp of the most recently detected press- or release-edge
    // (set inside update() whenever a Button reports wasPressed/wasReleased).
    // Latency-sensitive consumers can use this to measure the time between
    // the HAL detecting an edge and their own reaction completing.
    unsigned long getLastEdgeMs() const { return lastEdgeMs; }

    // Duration in ms of the most recently COMPLETED press (captured on the
    // release-edge). 0 until the first release. Useful for diagnostics like
    // verifying debounce timing or hold-duration distributions.
    unsigned long getLastReleasedDurationMs() const { return lastReleasedDurationMs; }

private:
    Adafruit_MCP23X17 mcp;
    bool mcpConnected = false;
    uint16_t mcpState = 0;
    uint16_t debouncedStates = 0;
    unsigned long lastEdgeMs = 0;
    unsigned long lastReleasedDurationMs = 0;

    std::vector<Button> buttons;
    unsigned long lastHardwareRead = 0;

    EventCallback callback = nullptr;

    // --- Helpers ---

    // Returns the current physical state of a button's pin (true = pressed).
    // Reads from the cached mcpState for MCP pins, or digitalRead() for ESP pins.
    bool readPhysical(const Button& btn) const;

    // Linear lookup by logical event. Returns nullptr if not found.
    // All ControlEvent-keyed queries (isPressed/getDuration/...) go through this.
    const Button* findByEvent(ControlEvent ev) const;
};