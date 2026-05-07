/**
 * Project: Arcade Controller V0.2
 * File: SoundManager.h
 * Description: Manages audio effects using ESP32 hardware PWM (LEDC).
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"

//Defines available sound effects for the system.
enum class SoundEffect {
    NONE,
    STARTUP,
    CLICK,
    LASER,
    EXPLOSION
};

class SoundManager {
private:
    uint8_t _pin;
    uint8_t _channel;
    uint8_t _volume;

    SoundEffect _currentEffect;
    unsigned long _startTime;
    int _duration;

    // --- Internal synthesis methods ---
    void updateLaser();
    void updateClick();
    void updateStartup();
    void updateExplosion();

public:

    //Constructor for SoundManager.
    SoundManager(uint8_t pin = PinConfig::SOUND_PWM_PIN, uint8_t channel = PinConfig::SOUND_CHANNEL);

    // Initializes the hardware PWM channel.
    void begin();

    // Starts playing a specific sound effect.
    void play(SoundEffect effect);

    // Processes the active sound effect. Must be called in the main loop.
    void update(); 

    // Immediately stops any currently playing sound.
    void stop();

    // Sets the system volume.
    void setVolume(uint8_t vol) { _volume = vol; }
};