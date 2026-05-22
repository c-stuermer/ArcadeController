/**
 * Project: Arcade Controller V1.3
 * File: InputHandler.cpp
 * Description: Software-side input processing. The hardware layer
 *              (InputReader) is injected so this class is fully
 *              decoupled from GPIO/I2C details.
 */

#include "InputHandler.h"

InputHandler::InputHandler(IInputReader* reader)
    : reader(reader)
{
    // The input lists live in Config.h (single source of truth). Each
    // input gets one Button entry; the Button is now identified solely
    // by its ControlEvent — its bit position in the raw bitmap equals
    // the numeric value of that ControlEvent.
    buttons.reserve(sizeof(PinConfig::ARCADE_INPUTS)   / sizeof(PinConfig::ARCADE_INPUTS[0])
                  + sizeof(PinConfig::JOYSTICK_INPUTS) / sizeof(PinConfig::JOYSTICK_INPUTS[0]));

    // --- Arcade Buttons --- (no hold-off: tapped quickly, chatter not observed)
    for (const auto& in : PinConfig::ARCADE_INPUTS) {
        buttons.emplace_back(in.event,
                             PinConfig::DEFAULT_DEBOUNCE_MS,
                             PinConfig::ARCADE_RELEASE_HOLDOFF_MS);
    }

    // --- Joystick --- (hold-off active: suppresses pre-snap microswitch chatter)
    for (const auto& in : PinConfig::JOYSTICK_INPUTS) {
        buttons.emplace_back(in.event,
                             PinConfig::DEFAULT_DEBOUNCE_MS,
                             PinConfig::JOYSTICK_RELEASE_HOLDOFF_MS);
    }
}

void InputHandler::update() {
    // Polling rate limit (1ms) to avoid saturating the I2C bus.
    if (millis() - lastHardwareRead < 1) return;
    lastHardwareRead = millis();

    // 1. Pull the raw bitmap once per tick. Bit N = ControlEvent N pressed.
    const uint16_t raw = reader->readRaw();

    // 2. Debounce each button against its bit slice, accumulate, dispatch edges.
    uint16_t newStates = 0;
    for (auto& btn : buttons) {
        const bool pressed = (raw >> static_cast<int>(btn.eventId)) & 1;
        btn.update(pressed);

        if (btn.isPressed()) {
            newStates |= (1u << static_cast<int>(btn.eventId));
        }

        // Record the edge timestamp once per detected transition (consumed
        // by latency-sensitive consumers via getLastEdgeMs()).
        if (btn.wasPressed() || btn.wasReleased()) {
            lastEdgeMs = millis();
        }

        // On the release-edge, capture how long the press lasted.
        if (btn.wasReleased()) {
            lastReleasedDurationMs = btn.getLastPressDuration();
        }

        if (btn.wasPressed()  && callback) callback(btn.eventId, EventType::PRESSED);
        if (btn.wasReleased() && callback) callback(btn.eventId, EventType::RELEASED);
    }
    debouncedStates = newStates;
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
