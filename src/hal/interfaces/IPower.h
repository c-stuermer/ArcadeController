/**
 * Project: Arcade Controller V1.3
 * File: IPower.h
 * Description: Abstract interface for the power subsystem. Defines the
 *              contract that any concrete PowerManager implementation
 *              must fulfil. Reset is wired to the ESP32 EN-pin and is
 *              therefore intentionally absent from this interface.
 */

#pragma once
#include <Arduino.h>

class IPower {
public:
    virtual ~IPower() = default;

    // --- Battery ---
    virtual float getBatteryVoltage()    const = 0;
    virtual int   getBatteryPercentage() const = 0;
};
