/**
 * Project: Arcade Controller V1.3
 * File: ISound.h
 * Description: Abstract interface for the audio subsystem. Defines the
 *              contract that any concrete SoundManager implementation must
 *              fulfil. The SoundEffect enum lives here because it is part
 *              of the interface (the parameter type of play()).
 */

#pragma once
#include <Arduino.h>

// Defines available sound effects for the system.
enum class SoundEffect {
    NONE,
    STARTUP,
    CLICK,
    LASER,
    EXPLOSION
};

class ISound {
public:

    virtual ~ISound() = default;

    virtual void play(SoundEffect effect)    = 0;
    virtual void stop()                      = 0;
    virtual void setVolume(uint8_t vol)      = 0;
};
