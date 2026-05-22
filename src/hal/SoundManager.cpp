/**
 * Project: Arcade Controller V1.3
 * File: SoundManager.cpp
 * Description: Implementation of audio effect synthesis.
 */

#include "SoundManager.h"

SoundManager::SoundManager(uint8_t pin, uint8_t channel)
    : pin(pin), channel(channel), volume(1), currentEffect(SoundEffect::NONE),
      startTime(0), duration(0) {}

void SoundManager::begin() {
    // Initialize the ESP32 PWM channel (LEDC)
    ledcSetup(channel, 2000, 8);
    ledcAttachPin(pin, channel);
    stop();
}

void SoundManager::play(SoundEffect effect) {
    if (volume == 0) return;   // Muted

    currentEffect = effect;
    startTime     = millis();

    // Set duration for each effect. CLICK fires its tone here so the
    // 15 ms blip is tight against the user input, while update() handles
    // termination (and ongoing synthesis for the other effects).
    switch (effect) {
        case SoundEffect::CLICK:
            duration = 15;
            ledcWriteTone(channel, 2500); // Fire immediately for tight UI feedback
            break;
        case SoundEffect::LASER:     duration = 300; break;
        case SoundEffect::STARTUP:   duration = 600; break;
        case SoundEffect::EXPLOSION: duration = 400; break;
        default:                     duration = 0;   break;
    }
}

void SoundManager::stop() {
    ledcWriteTone(channel, 0); // Stops the oscillation immediately
    currentEffect = SoundEffect::NONE;
}

void SoundManager::update() {
    if (currentEffect == SoundEffect::NONE) return;

    const unsigned long elapsed = millis() - startTime;

    // End the effect if the duration has expired
    if (elapsed > (unsigned long)duration) {
        stop();
        return;
    }

    // Synthesize sound based on elapsed time
    switch (currentEffect) {
        case SoundEffect::LASER:     updateLaser();     break;
        case SoundEffect::CLICK:     updateClick();     break;
        case SoundEffect::STARTUP:   updateStartup();   break;
        case SoundEffect::EXPLOSION: updateExplosion(); break;
        default: break;
    }
}

void SoundManager::updateLaser() {
    // Falling frequency: from 3000 Hz down to 500 Hz
    const int freq = map(millis() - startTime, 0, duration, 3000, 500);
    ledcWriteTone(channel, freq);
}

void SoundManager::updateClick() {
    // Constant high-frequency tone; play() already fired it, this just
    // re-asserts it for the remainder of the 15 ms window.
    ledcWriteTone(channel, 2500);
}

void SoundManager::updateStartup() {
    // Rising frequency (Retro coin/1-up sound)
    const unsigned long elapsed = millis() - startTime;
    const int freq = (elapsed < (unsigned long)duration / 2) ? 1200 : 2400;
    ledcWriteTone(channel, freq);
}

void SoundManager::updateExplosion() {
    // Simulate noise with random low frequencies
    ledcWriteTone(channel, random(50, 400));
}
