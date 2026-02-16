/**
 * Project: Arcade Controller V0.1
 * File: Button.h
 * Description: Handles software debouncing and state timing for buttons.
 */

#pragma once
#include <Arduino.h>

class Button {
public:
    // Constructor with default debounce time
    Button(unsigned long debounceMs = 20);

    // Call this in the main loop with the raw physical reading
    void update(bool currentReading);
    
    // State Queries
    bool isPressed() const;
    bool wasPressed() const;
    bool wasReleased() const;

    // Returns how long the button is currently being held (0 if released)
    unsigned long getActiveDuration() const;

    // Returns the duration of the LAST completed press (stored on release)
    unsigned long getLastPressDuration() const;

private:
    unsigned long debounceDelay;
    unsigned long lastDebounceTime = 0;
    
    // States
    bool debouncedState = false;       // The stable state
    bool lastDebouncedState = false;   // State from previous frame (for edge detection)
    
    // Timing
    unsigned long pressStartTime = 0;
    unsigned long _lastPressDuration = 0; 
};