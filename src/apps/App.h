/**
 * Project: Arcade Controller V1.1
 * File: apps/App.h
 * Description: Abstract base class for all applications.
 */

#pragma once
#include "../config/Config.h"

class ISystem;

class App {
protected:
    ISystem* system;

public:
    explicit App(ISystem* sys) : system(sys) {}
    virtual ~App() = default;

    virtual void start()  = 0;
    virtual void update() = 0;
    virtual void stop()   {}

    virtual void onInput(ControlEvent ev, EventType type) = 0;
};
