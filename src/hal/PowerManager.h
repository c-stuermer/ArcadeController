/**
 * Project: Arcade Controller V1.2
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

private:
    // Internal LED control. Toggled by begin() (on) and enterDeepSleep() (off).
    // No external caller needs this, so it stays out of the public interface.
    void  setSystemLedState(bool on);

    void  turnOffPeripherals();
    float readBatteryVoltage();
    int   calcBatteryPercentage(float volts);

private:
    const int batteryPin    = PinConfig::BATTERY_AD.pin;
    const int switchPwrPin  = PinConfig::POWER.pin;
    const int systemLedPin  = PinConfig::SYSTEM_LED.pin;

    const int backlightPin  = PinConfig::DISP_BLK;
    const int displayRstPin = PinConfig::DISP_RST;

    float batteryVoltage    = 0.0f;
    int   batteryPercentage = 0;
    unsigned long lastBatteryUpdate = 0;

    // --- Battery model constants ---
    static constexpr unsigned long BATTERY_UPDATE_MS = 1000;
    static constexpr float BATTERY_FULL_V  = 4.15f;   // 100% threshold
    static constexpr float BATTERY_EMPTY_V = 3.30f;   //   0% threshold
    static constexpr float VDIV_RATIO      = 2.0f;    // External voltage divider on the battery sense pin

    // --- Battery smoothing (moving average over SAMPLES ADC reads) ---
    static constexpr int SAMPLES = 10;
    int adcBuffer[SAMPLES];
    int bufferIndex = 0;
};
