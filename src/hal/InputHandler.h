/**
 * Project: Arcade Controller V1.3
 * File: InputHandler.h
 * Description: Software-side input processing. Pulls the raw bitmap from
 *              an injected IInputReader, debounces each ControlEvent
 *              individually via a Button, and dispatches edge events to
 *              a registered callback.
 *
 *              V1.3 change:
 *                The hardware boundary (MCP23017, ESP GPIO read, pin
 *                modes, I2C init) moved into InputReader. InputHandler
 *                now sees only IInputReader::readRaw() and is therefore
 *                fully decoupled from the physical wiring.
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>

#include "../config/Config.h"
#include "../driver/Button.h"
#include "interfaces/IInputReader.h"
#include "interfaces/IInputH.h"

class InputHandler : public IInputH {
public:
    // Define callback type (ControlEvent, PRESSED/RELEASED)
    using EventCallback = std::function<void(ControlEvent, EventType)>;

    explicit InputHandler(IInputReader* reader);

    // Polls the input bitmap and triggers events. Must be called in the main loop.
    void update();

    // Registers the callback function for input events
    void onEvent(EventCallback cb) { callback = cb; }

    // --- IInputH interface ---
    bool          isPressed(ControlEvent ev)            override;
    unsigned long getDuration(ControlEvent ev)          override;
    unsigned long getLastPressDuration(ControlEvent ev) override;

    // Debounced bitmap of all logical inputs. Bit position N corresponds to
    // the ControlEvent enum value N (e.g. bit 0 = BTN_A, bit 10 = JOY_UP).
    uint16_t      getDebouncedStates() const override { return debouncedStates; }

    // millis() timestamp of the most recently detected press- or release-edge.
    unsigned long getLastEdgeMs() const override { return lastEdgeMs; }

    // Duration in ms of the most recently COMPLETED press (captured on
    // the release-edge). 0 until the first release.
    unsigned long getLastReleasedDurationMs() const override { return lastReleasedDurationMs; }

private:
    // Raw bitmap source (non-owning). InputHandler never touches hardware
    // directly — every physical read goes through readRaw().
    IInputReader* reader;

    uint16_t      debouncedStates        = 0;
    unsigned long lastEdgeMs             = 0;
    unsigned long lastReleasedDurationMs = 0;

    std::vector<Button> buttons;
    unsigned long       lastHardwareRead = 0;

    EventCallback callback = nullptr;

    // Linear lookup by logical event. Returns nullptr if not found.
    const Button* findByEvent(ControlEvent ev) const;
};