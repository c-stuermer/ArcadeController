/**
 * Project: Arcade Controller V0.1
 * File: SettingsManager.h
 * Description: Persistent storage wrapper using ESP32 Preferences.
 */

#pragma once
#include <Preferences.h>

class SettingsManager {
    private:
        Preferences prefs;
    public:
        void begin() {
            // "arcade" is the namespace (max 15 chars). 
            // false = Read/Write mode
            prefs.begin("arcade", false); 
        }

        // --- VOLUME ---
        void setVolume(int vol) {
            // Clamp 0-100
            if (vol < 0) vol = 0; 
            if (vol > 100) vol = 100;
            prefs.putInt("volume", vol); // Saves immediately
        }

        int getVolume() {
            // 50 is default if key doesn't exist
            return prefs.getInt("volume", 50); 
        }

        // --- BRIGHTNESS ---
        void setBrightness(int bright) {
            if (bright < 0) bright = 0;
            if (bright > 255) bright = 255;
            prefs.putInt("brightness", bright);
        }

        int getBrightness() {
            return prefs.getInt("brightness", 255); // Default: Bright
        }

        // --- SOUND ENABLED ---
        void setSoundEnabled(bool enabled) {
            prefs.putBool("soundOn", enabled);
        }

        bool isSoundEnabled() {
            return prefs.getBool("soundOn", true); // Default: On
        }
};