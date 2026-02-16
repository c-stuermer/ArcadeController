/**
 * Project: Arcade Controller V0.1
 * File: SoundManager.cpp
 * Description: Implementation of non-blocking sound synthesis.
 */

#include "SoundManager.h"

SoundManager::SoundManager(uint8_t dacPin) 
    : _dacPin(dacPin), _enabled(false), _volume(200), _currentEffect(SoundEffect::NONE) {}

void SoundManager::begin() {
    // DAC pin does not need pinMode, dacWrite handles it
    _enabled = true;
    // Prevent "popping" noise at startup
    dacWrite(_dacPin, 0);
}

void SoundManager::play(SoundEffect effect) {
    if (!_enabled) return;
    _currentEffect = effect;
    _step = 0;
    _startTime = millis();
}

void SoundManager::update() {
    if (_currentEffect == SoundEffect::NONE) return;

    // Use micros() for better audio resolution
    unsigned long now = micros();

    switch (_currentEffect) {
        case SoundEffect::LASER:     updateLaser(); break;
        case SoundEffect::CLICK:     updateClick(); break;
        case SoundEffect::STARTUP:   updateStartup(); break;
        case SoundEffect::EXPLOSION: updateExplosion(); break;
        default: break;
    }
}

// --- Sound Generators (Non-Blocking) ---

void SoundManager::updateLaser() {
    // Simulates a sawtooth wave falling in frequency
    // _step counts up (Time)
    
    _step++;
    // Speed of pitch decay
    int pitch = 100 + (_step / 20); 
    
    if (_step > 4000) { // Duration approx 200ms (dependent on loop speed)
        _currentEffect = SoundEffect::NONE;
        dacWrite(_dacPin, 0);
        return;
    }

    // Sawtooth Waveform: (Time % Period) * Volume
    uint8_t val = (_step % pitch) * (_volume / (float)pitch);
    dacWrite(_dacPin, val);
}

void SoundManager::updateClick() {
    _step++;
    if (_step > 200) { 
        _currentEffect = SoundEffect::NONE;
        dacWrite(_dacPin, 0);
        return;
    }
    // Simple square wave impulse
    dacWrite(_dacPin, (_step < 100) ? _volume : 0);
}

void SoundManager::updateStartup() {
    _step++;
    // Frequency rises (Pitch value gets smaller)
    int pitch = 80 - (_step / 100); 
    if (pitch < 10) pitch = 10;

    if (_step > 5000) {
        _currentEffect = SoundEffect::NONE;
        dacWrite(_dacPin, 0);
        return;
    }
    
    // Square wave with rising frequency
    uint8_t val = ((_step / pitch) % 2) == 0 ? _volume : 0;
    dacWrite(_dacPin, val);
}

void SoundManager::updateExplosion() {
    _step++;
    if (_step > 3000) {
        _currentEffect = SoundEffect::NONE;
        dacWrite(_dacPin, 0);
        return;
    }
    
    // Pseudo-Random Noise
    // Amplitude decreases towards the end (Fade out)
    float fade = 1.0 - ((float)_step / 3000.0);
    uint8_t val = random(0, _volume * fade);
    dacWrite(_dacPin, val);
}