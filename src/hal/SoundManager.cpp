/**
 * Project: Arcade Controller V0.2
 * File: SoundManager.cpp
 * Description: Implementation of audio effect synthesis.
 */

#include "SoundManager.h"

SoundManager::SoundManager(uint8_t pin, uint8_t channel) 
    : _pin(pin), _channel(channel), _currentEffect(SoundEffect::NONE), _volume(1) {}

void SoundManager::begin() {
    // Initialize the ESP32 PWM channel (LEDC)
    ledcSetup(_channel, 2000, 8); 
    ledcAttachPin(_pin, _channel);
    stop();
}

void SoundManager::play(SoundEffect effect) {
    // Mute check
    if (_volume == 0) { return; }

    _currentEffect = effect;
    _startTime = millis();

    // Special case for CLICK: handle immediately without needing ongoing update()
    if (effect == SoundEffect::CLICK) {
        ledcWriteTone(_channel, 2500);
        delay(15); // 15ms is perfectly sufficient for a clean, crisp "tick"
        ledcWriteTone(_channel, 0); // Turn off immediately
        _currentEffect = SoundEffect::NONE;
        return; // We are done, update() doesn't need to process the click anymore
    }

    // Set duration for continuous effects
    switch (effect) {
        case SoundEffect::LASER:     _duration = 300;  break;
        case SoundEffect::STARTUP:   _duration = 600;  break;
        case SoundEffect::EXPLOSION: _duration = 400;  break;
        default:                     _duration = 0;    break;
    }
}

void SoundManager::stop() {
    ledcWriteTone(_channel, 0); // Stops the oscillation immediately
    _currentEffect = SoundEffect::NONE;
}

void SoundManager::update() {
    if (_currentEffect == SoundEffect::NONE) return;

    unsigned long elapsed = millis() - _startTime;

    // End the effect if the duration has expired
    if (elapsed > _duration) {
        stop();
        return;
    }

    // Synthesize sound based on elapsed time
    switch (_currentEffect) {
        case SoundEffect::LASER:     updateLaser(); break;
        case SoundEffect::CLICK:     updateClick(); break;
        case SoundEffect::STARTUP:   updateStartup(); break;
        case SoundEffect::EXPLOSION: updateExplosion(); break;
        default: break;
    }
}

void SoundManager::updateLaser() {
    // Falling frequency: from 3000Hz down to 500Hz
    int freq = map(millis() - _startTime, 0, _duration, 3000, 500);
    ledcWriteTone(_channel, freq);
}

void SoundManager::updateClick() {
    // Short, high-frequency pulse (mostly handled in play() already)
    ledcWriteTone(_channel, 2500);
}

void SoundManager::updateStartup() {
    // Rising frequency (Retro coin/1-up sound)
    unsigned long elapsed = millis() - _startTime;
    int freq = (elapsed < _duration / 2) ? 1200 : 2400;
    ledcWriteTone(_channel, freq);
}

void SoundManager::updateExplosion() {
    // Simulate noise with random low frequencies
    ledcWriteTone(_channel, random(50, 400));
}