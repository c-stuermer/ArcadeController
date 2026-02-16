/**
 * Project: Arcade Controller V0.1
 * File: Button.cpp
 * Description: Implementation of debounce logic.
 */

#include "Button.h"

Button::Button(unsigned long debounceMs) : debounceDelay(debounceMs) {
    // Initial state setup if needed
}

void Button::update(bool currentReading) {
    // Save the state from the PREVIOUS frame to detect edges later
    lastDebouncedState = debouncedState; 

    // 1. Debounce Timer Check
    // If we are within the debounce "dead zone", ignore changes.
    if ((millis() - lastDebounceTime) < debounceDelay) {
        return; 
    }

    // 2. State Change Detection
    // If the timer has expired and the physical reading is different from our stable state:
    if (currentReading != debouncedState) {
        lastDebounceTime = millis();    // Reset timer
        debouncedState = currentReading; // Accept new state immediately

        // --- LOGIC: Button Released ---
        if (debouncedState == false) {
            // Calculate and save how long it was held
            _lastPressDuration = millis() - pressStartTime;
        }

        // --- LOGIC: Button Pressed ---
        if (debouncedState == true) {
            // Reset duration and start timer
            pressStartTime = millis();
            _lastPressDuration = 0; 
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