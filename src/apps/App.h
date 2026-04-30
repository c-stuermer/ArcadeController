/**
 * Project: Arcade Controller V0.2
 * File: App.h
 * Description: Abstract base class for all applications.
 * Note: Consider making ArcadeController an interface to avoid circular dependencies.
 */

#pragma once
#include "../config/Config.h"

// Forward declaration: We tell the compiler "The class ArcadeController exists",
// but we don't include it here to avoid circular dependency loops.
class ArcadeController; 

class App {
protected:
    // Access to hardware and system resources
    ArcadeController* system; 

public:
    explicit App(ArcadeController* sys) : system(sys) {}
    virtual ~App() = default;

    // --- Lifecycle Methods ---
    
    // Setup when the app becomes active
    virtual void start() = 0;       
    
    // Main loop logic
    virtual void update() = 0;      
    
    // Cleanup when the app is closed
    virtual void stop() {}          

    // --- Event Handling ---
    
    // Processes incoming hardware control events
    virtual void onInput(ControlEvent ev, EventType type) = 0;
};