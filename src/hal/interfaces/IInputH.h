/**
 * Project: Arcade Controller V1.3
 * File: IInputH.h
 * Description: Abstract interface for the InputHandler — the read-only
 *              query surface that apps see. Owners (composition root /
 *              AppManager) still hold the concrete InputHandler so they
 *              can wire the event callback and drive update(); apps
 *              never see those internals.
 */

#pragma once
#include <Arduino.h>
#include "../../config/Config.h"   // ControlEvent

class IInputH {
public:
    virtual ~IInputH() = default;

    // --- State Checks ---
    virtual bool          isPressed(ControlEvent ev)              = 0;

    // --- Duration Checks ---
    virtual unsigned long getDuration(ControlEvent ev)            = 0;
    virtual unsigned long getLastPressDuration(ControlEvent ev)   = 0;

    // --- Bulk Snapshot ---
    // Debounced bitmap. Bit N corresponds to ControlEvent N.
    virtual uint16_t      getDebouncedStates() const              = 0;

    // --- Latency / diagnostics ---
    virtual unsigned long getLastEdgeMs() const                   = 0;
    virtual unsigned long getLastReleasedDurationMs() const       = 0;
};
