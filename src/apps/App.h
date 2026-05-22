/**
 * Project: Arcade Controller V1.3
 * File: apps/App.h
 * Description: Abstract base class for all applications.
 *
 *              V1.3: Apps are interface-injected by the composition root.
 *              The base class is intentionally minimal — start/update/stop
 *              and onInput — every other dependency (display, sound,
 *              settings, input, ...) is held by the concrete subclass as
 *              an interface pointer.
 */

#pragma once
#include "../config/Config.h"

class App {
public:
    App()          = default;
    virtual ~App() = default;

    virtual void start()  = 0;
    virtual void update() = 0;
    virtual void stop()   {}

    virtual void onInput(ControlEvent ev, EventType type) = 0;
};
