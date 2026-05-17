/**
 * Project: Arcade Controller V1.2
 * File: Button.cpp
 * Description: Implementation of eager-press debounce logic.
 */

#include "Button.h"

Button::Button(PinType type,
               uint8_t pin,
               ControlEvent eventId,
               unsigned long debounceMs,
               unsigned long releaseHoldoffMs)
    : type(type),
      pin(pin),
      eventId(eventId),
      debounceDelay(debounceMs),
      releaseHoldoff(releaseHoldoffMs) {}

void Button::update(bool currentReading) {
    // Save the state from the PREVIOUS frame to detect edges later
    lastDebouncedState = debouncedState;

    // Reset the debounce timer as long as the physical signal is fluctuating
    if (currentReading != lastRawReading) {
        lastDebounceTime = millis();
    }
    lastRawReading = currentReading;

    // --- 1. EAGER PRESS (0 ms latency, but gated by post-release hold-off) ---
    // If the signal is active and we were previously considered 'released':
    if (currentReading == true && debouncedState == false) {
        // Post-release hold-off: ignore press edges that arrive within
        // 'releaseHoldoff' ms of the last accepted release. Prevents
        // pre-snap contact chatter on slow microswitch releases from
        // latching as a phantom press immediately after the real release.
        if (releaseTime != 0 && (millis() - releaseTime) < releaseHoldoff) {
            return;   // still in dead-zone, drop this would-be press edge
        }
        debouncedState    = true;           // Fire immediately!
        pressStartTime    = millis();
        lastPressDuration = 0;
    }
    // --- 2. DELAYED RELEASE (Stability Check) ---
    // If the signal is gone, but we are still considered 'pressed':
    else if (currentReading == false && debouncedState == true) {
        // ONLY accept the release if the signal has been uninterruptedly
        // false for the duration of 'debounceDelay'.
        if ((millis() - lastDebounceTime) >= debounceDelay) {
            debouncedState    = false;
            lastPressDuration = millis() - pressStartTime;
            releaseTime       = millis();   // start the post-release hold-off window
        }
    }
}

bool Button::isPressed() const {
    return debouncedState;
}

bool Button::wasPressed() const {
    // True only if it is NOW pressed, but was NOT pressed in the previous frame
    return (debouncedState == true && lastDebouncedState == false);
}

bool Button::wasReleased() const {
    // True only if it is NOW released, but WAS pressed in the previous frame
    return (debouncedState == false && lastDebouncedState == true);
}

unsigned long Button::getActiveDuration() const {
    if (debouncedState) {
        return millis() - pressStartTime;
    }
    return 0;
}

unsigned long Button::getLastPressDuration() const {
    return lastPressDuration;
}