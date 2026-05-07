/**
 * Project: Arcade Controller V0.2
 * File: Button.h
 * Description: Handles software debouncing and state timing for physical buttons.
 */

#pragma once
#include <Arduino.h>

class Button {
public:

    // Constructor with default debounce time.
    Button(unsigned long debounceMs = 15);

    //Call this in the main loop with the raw physical reading.
    void update(bool currentReading);
    
    // --- State Queries ---
    
    // Returns true if the button is currently considered pressed
    bool isPressed() const;
    
    // Returns true only in the exact frame the button goes from released to pressed
    bool wasPressed() const;
    
    // Returns true only in the exact frame the button goes from pressed to released
    bool wasReleased() const;

    // --- Timing Queries ---
    
    // Returns how long the button is currently being held down (0 if released)
    unsigned long getActiveDuration() const;

    // Returns the total duration of the LAST completed press (stored on release)
    unsigned long getLastPressDuration() const;

private:
    unsigned long debounceDelay;
    unsigned long lastDebounceTime = 0;
    
    // --- States ---
    bool debouncedState = false;        // The stable, calculated state
    bool lastDebouncedState = false;    // State from previous frame (for edge detection)
    bool lastRawReading = false;        // Last unfiltered physical signal
    
    // --- Timing ---
    unsigned long pressStartTime = 0;
    unsigned long _lastPressDuration = 0; 
};