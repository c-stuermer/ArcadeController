/**
 * Project: Arcade Controller V0.1
 * File: PowerManager.cpp
 * Description: Implementation of power management logic and ADC smoothing.
 */

#include "PowerManager.h"

PowerManager::PowerManager() {
    // Initialize buffer with zeros
    for(int i=0; i<SAMPLES; i++) adcBuffer[i] = 0;
}

void PowerManager::init() {
    pinMode(systemLedPin, OUTPUT);
    pinMode(I2cVccPin, OUTPUT);
    pinMode(switchPwrPin, INPUT_PULLUP);
    pinMode(batteryPin, INPUT);

    turnOnPeripherals();
    setSystemLedState(true); 

    // Initial buffer filling to prevent 0V readings at start
    for(int i=0; i<SAMPLES; i++) {
        adcBuffer[i] = analogReadMilliVolts(batteryPin);
        delay(2);
    }

    batteryVoltage = readBatteryVoltage();
    batteryPercentage = calcBatteryPercentage(batteryVoltage);
}

void PowerManager::update() {
    // Read battery only at defined intervals (1s)
    if (millis() - lastBatteryUpdate > batteryUpdateInterval) {
        batteryVoltage = readBatteryVoltage();
        batteryPercentage = calcBatteryPercentage(batteryVoltage);
        lastBatteryUpdate = millis();
    }
}

float PowerManager::readBatteryVoltage() {
    // Read new value into circular buffer
    adcBuffer[bufferIndex] = analogReadMilliVolts(batteryPin);
    bufferIndex = (bufferIndex + 1) % SAMPLES;

    // Calculate average
    long sum = 0;
    for(int i=0; i<SAMPLES; i++) sum += adcBuffer[i];
    float avgMv = (float)sum / SAMPLES;

    // Calculation: Millivolts -> Volts * Voltage Divider Factor (2.0)
    return (avgMv * 2.0f) / 1000.0f; 
}

int PowerManager::calcBatteryPercentage(float voltage) {
    // Clamp to logical limits (LiPo specifics)
    if (voltage >= 4.15f) return 100;
    if (voltage <= 3.3f) return 0;

    // Map: 3.3V (0%) to 4.15V (100%)
    long pct = map((long)(voltage * 100), 330, 415, 0, 100);
    return constrain(pct, 0, 100);
}

bool PowerManager::isUSBConnected() {
    // Simple heuristic: If voltage is > 4.2V, we are likely charging.
    return batteryVoltage > 4.20f;
}

bool PowerManager::isSwitchedOn() {
    return digitalRead(switchPwrPin) == LOW;
}

void PowerManager::setSystemLedState(bool on) {
    digitalWrite(systemLedPin, on ? HIGH : LOW);
}

void PowerManager::turnOnPeripherals() {
    digitalWrite(I2cVccPin, HIGH);
    delay(25); // Wait for peripherals to stabilize
}

void PowerManager::turnOffPeripherals() {
    // Cut power to Display & MCP Reset
    digitalWrite(I2cVccPin, LOW);
    
    // CRITICAL: Set I2C pins to INPUT.
    // This prevents parasitic power drain through internal protection diodes
    // or pull-up resistors while the peripherals are powered down.
    pinMode(PinConfig::I2C_SDA, INPUT);
    pinMode(PinConfig::I2C_SCL, INPUT);
}

void PowerManager::enterDeepSleep() {
    Serial.println(">>> ENTERING DEEP SLEEP <<<");
    Serial.flush();
    
    setSystemLedState(false);
    turnOffPeripherals();
    
    // Configure wake up source: Physical Switch (LOW level wakeup)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)switchPwrPin, 0);
    
    delay(100);
    esp_deep_sleep_start();
}