/**
 * Project: Arcade Controller V0.2
 * File: PowerManager.h
 * Description: Manages battery monitoring, sleep modes, and pin states (Hardware Logic).
 */

#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class PowerManager {
public:
    PowerManager(); 

    void begin();
    void update();

    // --- Getters ---
    float getBatteryVoltage() const { return batteryVoltage; }
    int getBatteryPercentage() const { return batteryPercentage; }
    
    // --- Status --- 
    bool isSwitchedOn();

    // --- Actions ---
    void enterDeepSleep();
    void setSystemLedState(bool on);

private: 
    // --- Internal Helpers ---
    void turnOffPeripherals();
    float readBatteryVoltage();
    int calcBatteryPercentage(float volts);

private: 
    // --- Config & Pins ---
    const int batteryPin   = PinConfig::BATTERY_AD.pin;
    const int switchPwrPin = PinConfig::POWER.pin;
    const int systemLedPin = PinConfig::SYSTEM_LED.pin;
    
    // Display Power Control Pins
    const int backlightPin = PinConfig::DISP_BLK; 
    const int displayRstPin = PinConfig::DISP_RST; 

    // --- Battery State ---
    float batteryVoltage = 0.0;
    int batteryPercentage = 0;
    unsigned long lastBatteryUpdate = 0;
    const unsigned long batteryUpdateInterval = 1000;

    // --- ADC Smoothing ---
    static const int SAMPLES = 10;
    int adcBuffer[SAMPLES];
    int bufferIndex = 0;
};