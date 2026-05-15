/**
 * Project: Arcade Controller V1.1
 * File: hal/PowerManager.h
 * Description: Battery monitoring, sleep modes, and the physical power
 *              switch. The reset button is wired to the ESP32 EN-pin
 *              (hardware reset), so it is invisible to software.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class PowerManager {
public:
    PowerManager();

    void begin();
    void update();

    // --- Battery ---
    float getBatteryVoltage()    const { return batteryVoltage; }
    int   getBatteryPercentage() const { return batteryPercentage; }

    // --- Power switch state ---
    bool isSwitchedOn();

    // --- Actions ---
    void enterDeepSleep();
    void setSystemLedState(bool on);

private:
    void  turnOffPeripherals();
    float readBatteryVoltage();
    int   calcBatteryPercentage(float volts);

private:
    const int batteryPin    = PinConfig::BATTERY_AD.pin;
    const int switchPwrPin  = PinConfig::POWER.pin;
    const int systemLedPin  = PinConfig::SYSTEM_LED.pin;

    const int backlightPin  = PinConfig::DISP_BLK;
    const int displayRstPin = PinConfig::DISP_RST;

    float batteryVoltage = 0.0f;
    int   batteryPercentage = 0;
    unsigned long lastBatteryUpdate = 0;
    const unsigned long batteryUpdateInterval = 1000;

    static const int SAMPLES = 10;
    int adcBuffer[SAMPLES];
    int bufferIndex = 0;
};
