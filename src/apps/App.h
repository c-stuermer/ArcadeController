/**
 * Project: Arcade Controller V0.1
 * File: App.h
 * Description: Abstract base class for all applications.
 */

#pragma once
#include "../config/Config.h"

// Forward declaration: We tell the compiler "The class ArcadeController exists",
// but we don't include it here to avoid circular dependency loops.
class ArcadeController; 

class App {
protected:
    ArcadeController* system; // Access to hardware

public:
    explicit App(ArcadeController* sys) : system(sys) {}
    virtual ~App() = default;

    // --- Lifecycle Methods ---
    virtual void start() = 0;       // Setup when app becomes active
    virtual void update() = 0;      // Main loop logic
    virtual void stop() {}          // Cleanup when app is closed

    // --- Event Handling ---
    virtual void onInput(ControlEvent ev, EventType type) = 0;
};