/**
 * Project: Arcade Controller V1.1
 * File: InputHandler.cpp
 * Description: Implementation of input scanning and debounce logic.
 */

#include "InputHandler.h"

struct ButtonMapEntry {
    HardwarePin hw;
    ControlEvent eventId;
};

static const ButtonMapEntry BUTTON_MAP[] = {

    // Arcade Buttons
    { PinConfig::ARCADE_A,      ControlEvent::BTN_A },
    { PinConfig::ARCADE_B,      ControlEvent::BTN_B },
    { PinConfig::ARCADE_X,      ControlEvent::BTN_X },
    { PinConfig::ARCADE_Y,      ControlEvent::BTN_Y },
    { PinConfig::ARCADE_L1,     ControlEvent::BTN_L1 },
    { PinConfig::ARCADE_R1,     ControlEvent::BTN_R1 },
    { PinConfig::ARCADE_L2,     ControlEvent::BTN_L2 },
    { PinConfig::ARCADE_R2,     ControlEvent::BTN_R2 },
    { PinConfig::ARCADE_SELECT, ControlEvent::BTN_SELECT },
    { PinConfig::ARCADE_START,  ControlEvent::BTN_START },
    
    // Joystick
    { PinConfig::JOYSTICK_UP,   ControlEvent::JOY_UP },
    { PinConfig::JOYSTICK_DOWN, ControlEvent::JOY_DOWN },
    { PinConfig::JOYSTICK_LEFT, ControlEvent::JOY_LEFT },
    { PinConfig::JOYSTICK_RIGHT,ControlEvent::JOY_RIGHT }

    // Note: The power switch is polled directly by PowerManager via digitalRead()
    // and is intentionally not part of this map.
};

InputHandler::InputHandler() {
    // Pre-allocate memory for all buttons to avoid reallocation overhead
    buttons.reserve(sizeof(BUTTON_MAP) / sizeof(ButtonMapEntry));
    unsigned long debounceMs = PinConfig::DEFAULT_DEBOUNCE_MS;

    // Initialize all buttons
    for (const auto& entry : BUTTON_MAP) {
        buttons.push_back({ 
            entry.hw,           // Hardware pin (ESP/MCP + Pin)
            Button(debounceMs), // Debounce logic class
            entry.eventId       // Event ID
        });
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
        if (btn.hw.type == PinType::MCP && mcpConnected) {
            // MCP pins: Input with internal pull-up
            mcp.pinMode(btn.hw.pin, INPUT_PULLUP);
        } 
        else if (btn.hw.type == PinType::ESP) {
            // ESP pins: Input with internal pull-up
            pinMode(btn.hw.pin, INPUT_PULLUP);
        }
    }
}

void InputHandler::update() {
    // Polling rate limit (1ms) to avoid saturating the I2C bus
    if (millis() - lastHardwareRead < 1) return; 
    lastHardwareRead = millis();    

    // 1. Hardware Read (Bulk read for the MCP is much more efficient)
    if (mcpConnected) {
        // Invert (~) because INPUT_PULLUP logic is LOW when pressed.
        // Result: 1 = Pressed, 0 = Released.
        mcpState = ~mcp.readGPIOAB(); 
    }

    // 2. Iterate through all configured buttons
    uint16_t newStates = 0;
    for (auto &btn : buttons) {
        bool isPhysicalPressed = false;

        if (btn.hw.type == PinType::MCP && mcpConnected) {
            // Bitmask check: Is the specific bit at 'pin' set?
            isPhysicalPressed = (mcpState >> btn.hw.pin) & 1;
        }
        else if (btn.hw.type == PinType::ESP) {
            // ESP pins: LOW = Pressed (Pull-up logic)
            isPhysicalPressed = (digitalRead(btn.hw.pin) == LOW);
        }

        // 3. Feed the physical reading into the debounce logic
        btn.logic.update(isPhysicalPressed);

        // 4. Maintain the debounced-state bitmap (bit position = ControlEvent value)
        if (btn.logic.isPressed()) {
            newStates |= (1u << static_cast<int>(btn.eventId));
        }

        // 5. Process Events (Edge Detection)
        if (btn.logic.wasPressed()) {
            if (_callback) _callback(btn.eventId, EventType::PRESSED);
        }

        if (btn.logic.wasReleased()) {
            if (_callback) _callback(btn.eventId, EventType::RELEASED);
        }
    }
    _debouncedStates = newStates;
}

bool InputHandler::isPressed(ControlEvent ev) {
    for (const auto &btn : buttons) {
        if (btn.eventId == ev) return btn.logic.isPressed();
    }
    return false;
}

bool InputHandler::isPressed(HardwarePin pinConfig) {
    for (const auto &btn : buttons) {
        if (btn.hw.pin == pinConfig.pin && btn.hw.type == pinConfig.type) {
            return btn.logic.isPressed();
        }
    }
    return false;
}

unsigned long InputHandler::getDuration(ControlEvent ev) {
    for (const auto &btn : buttons) {
        if (btn.eventId == ev) {
            return btn.logic.getActiveDuration();
        }
    }
    return 0;
}

unsigned long InputHandler::getLastPressDuration(ControlEvent ev) {
    for (const auto &btn : buttons) {
        if (btn.eventId == ev) {
            return btn.logic.getLastPressDuration();
        }
    }
    return 0;
}