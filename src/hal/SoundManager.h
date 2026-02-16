/**
 * Project: Arcade Controller V0.1
 * File: SoundManager.h
 * Description: Generates simple retro sound effects via DAC.
 */

#pragma once
#include <Arduino.h>

enum class SoundEffect {
    NONE,
    STARTUP,    // Rising "Coin" sound
    CLICK,      // Short "Blip"
    LASER,      // Falling "Pew" sound (Sawtooth)
    EXPLOSION   // White Noise
};

class SoundManager {
private:
    uint8_t _dacPin;
    bool _enabled;
    uint8_t _volume; // 0-255 (Scaling)

    // Playback State
    SoundEffect _currentEffect;
    unsigned long _lastUpdate;
    unsigned long _startTime;
    int _step; 
    
    // Generators
    void updateLaser();
    void updateClick();
    void updateStartup();
    void updateExplosion();

public:
    SoundManager(uint8_t dacPin = 25);
    void begin();
    void play(SoundEffect effect);
    void update(); // Must be called in main loop!
};