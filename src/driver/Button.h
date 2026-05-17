/**
 * Project: Arcade Controller V1.2
 * File: Button.h
 * Description: Self-contained button entity. Owns its hardware identity
 *              (PinType + pin), its logical identity (ControlEvent) and
 *              its debounce/timing state in a single flat object.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"   // PinType, ControlEvent

class Button {
public:
    // --- Identity (flat, public access by design) ---
    // Read frequently from the outside: InputHandler loops over a
    // std::vector<Button> and reads btn.type / btn.pin / btn.eventId
    // directly. Keeping them public removes getter ceremony; the
    // debounce/timing state below remains private.
    PinType      type;
    uint8_t      pin;
    ControlEvent eventId;

    // Constructor: full identity + optional debounce and release-holdoff overrides.
    Button(PinType type,
           uint8_t pin,
           ControlEvent eventId,
           unsigned long debounceMs        = 25,
           unsigned long releaseHoldoffMs  = 20);

    // Call this in the main loop with the raw physical reading.
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
    // --- Debounce config ---
    unsigned long debounceDelay;
    unsigned long releaseHoldoff;        // dead-zone after release in which no new press fires
    unsigned long lastDebounceTime = 0;

    // --- States ---
    bool debouncedState = false;        // The stable, calculated state
    bool lastDebouncedState = false;    // State from previous frame (for edge detection)
    bool lastRawReading = false;        // Last unfiltered physical signal

    // --- Timing ---
    unsigned long pressStartTime    = 0;
    unsigned long lastPressDuration = 0;
    unsigned long releaseTime       = 0; // millis() of last accepted release; 0 = none yet
};