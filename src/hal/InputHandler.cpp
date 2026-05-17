/**
 * Project: Arcade Controller V1.2
 * File: InputHandler.cpp
 * Description: Implementation of input scanning and debounce logic.
 */

#include "InputHandler.h"

InputHandler::InputHandler() {
    const unsigned long d   = PinConfig::DEFAULT_DEBOUNCE_MS;
    const unsigned long h_a = PinConfig::ARCADE_RELEASE_HOLDOFF_MS;    // 0  -> off
    const unsigned long h_j = PinConfig::JOYSTICK_RELEASE_HOLDOFF_MS;  // 50 -> pre-snap chatter mitigation

    // 14 buttons total (10 arcade + 4 joystick directions).
    // The power switch is polled directly by PowerManager via digitalRead()
    // and is intentionally not part of this list.
    buttons.reserve(14);

    // --- Arcade Buttons --- (no hold-off: tapped quickly, chatter not observed)
    buttons.emplace_back(PinConfig::ARCADE_A.type,      PinConfig::ARCADE_A.pin,      ControlEvent::BTN_A,      d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_B.type,      PinConfig::ARCADE_B.pin,      ControlEvent::BTN_B,      d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_X.type,      PinConfig::ARCADE_X.pin,      ControlEvent::BTN_X,      d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_Y.type,      PinConfig::ARCADE_Y.pin,      ControlEvent::BTN_Y,      d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_L1.type,     PinConfig::ARCADE_L1.pin,     ControlEvent::BTN_L1,     d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_R1.type,     PinConfig::ARCADE_R1.pin,     ControlEvent::BTN_R1,     d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_L2.type,     PinConfig::ARCADE_L2.pin,     ControlEvent::BTN_L2,     d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_R2.type,     PinConfig::ARCADE_R2.pin,     ControlEvent::BTN_R2,     d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_SELECT.type, PinConfig::ARCADE_SELECT.pin, ControlEvent::BTN_SELECT, d, h_a);
    buttons.emplace_back(PinConfig::ARCADE_START.type,  PinConfig::ARCADE_START.pin,  ControlEvent::BTN_START,  d, h_a);

    // --- Joystick --- (hold-off active: suppresses pre-snap microswitch chatter)
    buttons.emplace_back(PinConfig::JOYSTICK_UP.type,    PinConfig::JOYSTICK_UP.pin,    ControlEvent::JOY_UP,    d, h_j);
    buttons.emplace_back(PinConfig::JOYSTICK_DOWN.type,  PinConfig::JOYSTICK_DOWN.pin,  ControlEvent::JOY_DOWN,  d, h_j);
    buttons.emplace_back(PinConfig::JOYSTICK_LEFT.type,  PinConfig::JOYSTICK_LEFT.pin,  ControlEvent::JOY_LEFT,  d, h_j);
    buttons.emplace_back(PinConfig::JOYSTICK_RIGHT.type, PinConfig::JOYSTICK_RIGHT.pin, ControlEvent::JOY_RIGHT, d, h_j);
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