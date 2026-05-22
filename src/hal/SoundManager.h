/**
 * Project: Arcade Controller V1.3
 * File: SoundManager.h
 * Description: Manages audio effects using ESP32 hardware PWM (LEDC).
 *              Implements the ISound interface. The SoundEffect enum has
 *              moved into ISound.h so it lives with the contract that
 *              uses it.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"
#include "interfaces/ISound.h"

class SoundManager : public ISound {
public:
    // Constructor for SoundManager.
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
    void setVolume(uint8_t vol) override { volume = vol; }

private:
    uint8_t pin;
    uint8_t channel;
    uint8_t volume;

    SoundEffect   currentEffect;
    unsigned long startTime;
    int           duration;

    // --- Internal synthesis methods ---
    void updateLaser();
    void updateClick();
    void updateStartup();
    void updateExplosion();
};
