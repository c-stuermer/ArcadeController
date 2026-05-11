/**
 * Project: Arcade Controller V1.0
 * File: Button.cpp
 * Description: Implementation of eager-press debounce logic.
 */

#include "Button.h"

Button::Button(unsigned long debounceMs) : debounceDelay(debounceMs) {
    // Initial state setup if needed
}

void Button::update(bool currentReading) {
    // Save the state from the PREVIOUS frame to detect edges later
    lastDebouncedState = debouncedState; 

    // Reset the debounce timer as long as the physical signal is fluctuating
    if (currentReading != lastRawReading) {
        lastDebounceTime = millis();
    }
    lastRawReading = currentReading;

    // --- 1. EAGER PRESS (0 ms latency) ---
    // If the signal is active and we were previously considered 'released':
    if (currentReading == true && debouncedState == false) {
        debouncedState = true;           // Fire immediately!
        pressStartTime = millis();
        _lastPressDuration = 0; 
    }
    // --- 2. DELAYED RELEASE (Stability Check) ---
    // If the signal is gone, but we are still considered 'pressed':
    else if (currentReading == false && debouncedState == true) {
        // We ONLY accept the release if the signal has been uninterruptedly 
        // false for the duration of 'debounceDelay'.
        if ((millis() - lastDebounceTime) >= debounceDelay) {
            debouncedState = false;
            _lastPressDuration = millis() - pressStartTime;
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
    return _lastPressDuration;
}