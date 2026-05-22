/**
 * Project: Arcade Controller V1.3
 * File: Button.h
 * Description: Self-contained debounce entity. Owns its logical identity
 *              (ControlEvent) and its debounce/timing state.
 *
 *              V1.3 change:
 *                Button no longer carries its hardware identity (PinType,
 *                pin number). The InputReader is now solely responsible
 *                for translating pins into a ControlEvent-indexed bitmap;
 *                InputHandler feeds each Button the relevant bit. Button
 *                is purely a stateful debounce filter from this version.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"   // ControlEvent

class Button {
public:
    // --- Identity (flat, public access by design) ---
    // InputHandler iterates a std::vector<Button> and reads btn.eventId
    // directly. The debounce/timing state below remains private.
    ControlEvent eventId;

    // Constructor: logical identity + debounce/release-holdoff overrides.
    Button(ControlEvent eventId,
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
