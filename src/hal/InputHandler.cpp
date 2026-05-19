/**
 * Project: Arcade Controller V1.2
 * File: InputHandler.cpp
 * Description: Implementation of input scanning and debounce logic.
 */

#include "InputHandler.h"

InputHandler::InputHandler() {
    // The input lists live in Config.h (single source of truth). The power
    // switch is polled directly by PowerManager via digitalRead() and is
    // intentionally not part of this registry.
    buttons.reserve(sizeof(PinConfig::ARCADE_INPUTS)   / sizeof(PinConfig::ARCADE_INPUTS[0])
                  + sizeof(PinConfig::JOYSTICK_INPUTS) / sizeof(PinConfig::JOYSTICK_INPUTS[0]));

    // --- Arcade Buttons --- (no hold-off: tapped quickly, chatter not observed)
    for (const auto& in : PinConfig::ARCADE_INPUTS) {
        buttons.emplace_back(in.hw.type, 
                             in.hw.pin, 
                             in.event,
                             PinConfig::DEFAULT_DEBOUNCE_MS,
                             PinConfig::ARCADE_RELEASE_HOLDOFF_MS);
    }

    // --- Joystick --- (hold-off active: suppresses pre-snap microswitch chatter)
    for (const auto& in : PinConfig::JOYSTICK_INPUTS) {
        buttons.emplace_back(in.hw.type, 
                             in.hw.pin, 
                             in.event,
                             PinConfig::DEFAULT_DEBOUNCE_MS,
                             PinConfig::JOYSTICK_RELEASE_HOLDOFF_MS);
    }
}

void InputHandler::begin() {
    Wire.begin();
    Wire.setClock(400000); // Fast I2C Clock

    // Check for MCP23017 on the bus
    Wire.beginTransmission(PinConfig::MCP_ADDRESS);
    if (Wire.endTransmission() == 0) {
        Serial.println("[I2C] MCP23017 connected!");
        mcpConnected = true;
        mcp.begin_I2C(PinConfig::MCP_ADDRESS);
    } else {
        Serial.println("[I2C] MCP23017 NOT found!");
        mcpConnected = false;
    }

    // Set pin modes for all configured buttons
    for (auto &btn : buttons) {
        if (btn.type == PinType::MCP && mcpConnected) {
            // MCP pins: Input with internal pull-up
            mcp.pinMode(btn.pin, INPUT_PULLUP);
        }
        else if (btn.type == PinType::ESP) {
            // ESP pins: Input with internal pull-up
            pinMode(btn.pin, INPUT_PULLUP);
        }
    }
}

void InputHandler::update() {
    // Polling rate limit (1ms) to avoid saturating the I2C bus
    if (millis() - lastHardwareRead < 1) return;
    lastHardwareRead = millis();

    // 1. Bulk-read the MCP once per tick. Inverted because INPUT_PULLUP
    //    is LOW when pressed -> after ~: 1 = Pressed, 0 = Released.
    if (mcpConnected) {
        mcpState = ~mcp.readGPIOAB();
    }

    // 2. For each button: read physical, debounce, accumulate, dispatch edges.
    uint16_t newStates = 0;
    for (auto &btn : buttons) {
        btn.update(readPhysical(btn));

        if (btn.isPressed()) {
            newStates |= (1u << static_cast<int>(btn.eventId));
        }

        // Record the edge timestamp once per detected transition (consumed
        // by latency-sensitive consumers via getLastEdgeMs()).
        if (btn.wasPressed() || btn.wasReleased()) {
            lastEdgeMs = millis();
        }

        // On the release-edge, capture how long the press lasted. The Button
        // already updated its internal lastPressDuration inside update().
        if (btn.wasReleased()) {
            lastReleasedDurationMs = btn.getLastPressDuration();
        }

        if (btn.wasPressed()  && callback) callback(btn.eventId, EventType::PRESSED);
        if (btn.wasReleased() && callback) callback(btn.eventId, EventType::RELEASED);
    }
    debouncedStates = newStates;
}

bool InputHandler::readPhysical(const Button& btn) const {
    if (btn.type == PinType::MCP && mcpConnected) {
        // Bitmask check: Is the specific bit at 'pin' set in the cached read?
        return (mcpState >> btn.pin) & 1;
    }
    if (btn.type == PinType::ESP) {
        // ESP pins: LOW = Pressed (Pull-up logic)
        return digitalRead(btn.pin) == LOW;
    }
    return false;
}

const Button* InputHandler::findByEvent(ControlEvent ev) const {
    for (const auto& btn : buttons) {
        if (btn.eventId == ev) return &btn;
    }
    return nullptr;
}

bool InputHandler::isPressed(ControlEvent ev) {
    const Button* btn = findByEvent(ev);
    return btn && btn->isPressed();
}

unsigned long InputHandler::getDuration(ControlEvent ev) {
    const Button* btn = findByEvent(ev);
    return btn ? btn->getActiveDuration() : 0;
}

unsigned long InputHandler::getLastPressDuration(ControlEvent ev) {
    const Button* btn = findByEvent(ev);
    return btn ? btn->getLastPressDuration() : 0;
}