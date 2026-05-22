/**
 * Project: Arcade Controller V1.3
 * File: transport/GamepadOutput.h
 * Description: Abstract base class for gamepad transports.
 *
 *              V1.3 change:
 *                The old IGamepadOutput interface has been replaced by
 *                this abstract base class. The base class owns the
 *                shared dispatch logic (ControlEvent -> HID button index
 *                lookup, joystick state tracking, hat-direction
 *                computation) so a new transport (Switch2, USB, ...) only
 *                has to implement the small set of backend-specific hooks
 *                (doPressButton, doReleaseButton, doSendJoystickReport).
 *
 *                Template Method pattern: press()/release() are concrete
 *                non-virtual methods in the base; the hooks they call
 *                are pure virtual.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class GamepadOutput {
public:
    virtual ~GamepadOutput() = default;

    // --- Backend lifecycle / state (purely virtual) ---
    virtual void begin()              = 0;
    virtual bool isConnected()        = 0;
    virtual bool isAdvertising()      = 0;
    virtual void disconnect()         = 0;
    virtual void startAdvertising()   = 0;
    virtual void stopAdvertising()    = 0;
    virtual void setBatteryLevel(int level) = 0;

    // --- Shared dispatch (template method) ---
    // Every transport sees the same ControlEvent vocabulary; the base
    // class translates that to HID button indices / joystick reports and
    // calls into the backend hooks below.
    void press(ControlEvent event);
    void release(ControlEvent event);

protected:
    // --- Backend hooks (called by press/release in the base class) ---
    virtual void doPressButton(uint8_t hidIndex)   = 0;
    virtual void doReleaseButton(uint8_t hidIndex) = 0;
    virtual void doSendJoystickReport()            = 0;

    // --- Joystick state (shared, read by doSendJoystickReport) ---
    bool joyUp    = false;
    bool joyDown  = false;
    bool joyLeft  = false;
    bool joyRight = false;

private:
    // Internal helpers. Returns true if 'event' was a joystick direction
    // and was therefore handled here; false otherwise -> caller falls
    // through to the button-mapping path.
    bool handleJoystick(ControlEvent event, bool pressed);

    // Looks up the HID button index for a logical event. 0 if none.
    static uint8_t hidButtonFor(ControlEvent ev);
};
