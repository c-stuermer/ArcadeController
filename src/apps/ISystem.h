/**
 * Project: Arcade Controller V1.1
 * File: apps/ISystem.h
 * Description: Narrow service interface that the App layer uses to talk to
 *              the system. ArcadeController implements this interface.
 */

#pragma once
#include <Arduino.h>

class DisplayManager;
class SoundManager;
class PowerManager;
class SettingsManager;
class AppManager;
class InputHandler;

class ISystem {
public:
    virtual ~ISystem() = default;

    // --- Subsystem accessors ---
    virtual DisplayManager*  getDisplay()    = 0;
    virtual SoundManager*    getSound()      = 0;
    virtual PowerManager*    getPower()      = 0;
    virtual SettingsManager* getSettings()   = 0;
    virtual AppManager*      getAppManager() = 0;
    virtual InputHandler*    getInput()      = 0;

    // --- Cross-cutting setters (persist + apply) ---
    virtual void setBrightness(uint8_t level) = 0;
    virtual void setVolume(uint8_t level)     = 0;
    virtual void setBootMode(uint8_t mode)    = 0;
};
