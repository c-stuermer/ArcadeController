/**
 * Project: Arcade Controller V0.1
 * File: PowerManager.h
 * Description: Manages battery monitoring, power switching, and sleep modes.
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class PowerManager {
public:
    PowerManager();

    void init();
    void update();

    // --- Getters ---
    float getBatteryVoltage() const { return batteryVoltage; }
    int getBatteryPercentage() const { return batteryPercentage; }
    
    // --- Status ---
    // Returns true if voltage indicates charging (> 4.2V)
    bool isUSBConnected();  
    
    // Checks physical switch state (LOW = ON)
    bool isSwitchedOn();

    // --- Actions ---
    void enterDeepSleep();
    void setSystemLedState(bool on);

    // --- Peripheral Control (OLED & MCP) ---
    void turnOnPeripherals();
    void turnOffPeripherals();

private:
    // Configuration from PinConfig
    const int batteryPin   = PinConfig::BATTERY_AD;
    const int switchPwrPin = PinConfig::POWER.pin;
    const int systemLedPin = PinConfig::SYSTEM_LED.pin;
    const int I2cVccPin    = PinConfig::I2C_VCC;

    // Battery State
    float batteryVoltage = 0.0;
    int batteryPercentage = 0;
    unsigned long lastBatteryUpdate = 0;
    const unsigned long batteryUpdateInterval = 1000;

    // ADC Smoothing (Circular Buffer)
    static const int SAMPLES = 10;
    int adcBuffer[SAMPLES];
    int bufferIndex = 0;

    // Helpers
    float readBatteryVoltage();
    int calcBatteryPercentage(float volts);
};