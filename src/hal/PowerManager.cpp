/**
 * Project: Arcade Controller V0.2
 * File: PowerManager.cpp
 * Description: Handles power management and ghost-glow prevention during deep sleep.
 */

#include "PowerManager.h"
#include "driver/rtc_io.h" // REQUIRED for gpio_hold functions

PowerManager::PowerManager() {
    for(int i = 0; i < SAMPLES; i++) adcBuffer[i] = 0;
}

void PowerManager::begin() {
    Serial.println("[POWER] Begin");
    // 1. IMPORTANT: Release the "Hold"!
    // When waking up from deep sleep, backlight and reset pins are frozen.
    // We must release them before they can be assigned to new functions.
    gpio_hold_dis((gpio_num_t)backlightPin);
    gpio_hold_dis((gpio_num_t)displayRstPin);

    // 2. Configure standard pins
    pinMode(systemLedPin, OUTPUT);
    pinMode(switchPwrPin, INPUT_PULLUP);
    pinMode(batteryPin, INPUT);
    
    // Take over display pins for power management
    pinMode(backlightPin, OUTPUT);
    pinMode(displayRstPin, OUTPUT);

    // 3. Set initial states
    setSystemLedState(true); 

    // 4. Initial battery read (fill the smoothing buffer)
    for(int i = 0; i < SAMPLES; i++) {
        adcBuffer[i] = analogReadMilliVolts(batteryPin);
        delay(2);
    }
    batteryVoltage = readBatteryVoltage();
    batteryPercentage = calcBatteryPercentage(batteryVoltage);
}

void PowerManager::turnOffPeripherals() {
    // 1. Turn off backlight (remove PWM assignment and pull LOW)
    gpio_reset_pin((gpio_num_t)backlightPin); 
    pinMode(backlightPin, OUTPUT);
    digitalWrite(backlightPin, LOW); 
    
    // 2. Hard disable the display controller (hold reset LOW)
    digitalWrite(displayRstPin, LOW);

    // 3. Make I2C pins high-impedance to prevent current leaks
    pinMode(PinConfig::I2C_SDA, INPUT);
    pinMode(PinConfig::I2C_SCL, INPUT);
    
    // 4. Make SPI pins high-impedance so the display doesn't drain power
    pinMode(PinConfig::SPI_MOSI, INPUT); 
    pinMode(PinConfig::SPI_SCLK, INPUT);
    pinMode(PinConfig::DISP_CS, INPUT);
    pinMode(PinConfig::DISP_DC, INPUT);
    
    // 5. "GHOST GLOW" FIX: Freeze the current pin states
    gpio_hold_en((gpio_num_t)backlightPin);
    gpio_hold_en((gpio_num_t)displayRstPin);

    // Turn off built-in LED and hold its state
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    gpio_hold_en((gpio_num_t)LED_BUILTIN);
}

void PowerManager::enterDeepSleep() {
    Serial.println("[POWER] ENTERING DEEP SLEEP");
    Serial.flush();
    
    setSystemLedState(false);
    
    // Shut down peripherals and activate GPIO hold
    turnOffPeripherals(); 
    
    // Tell the ESP32 to keep the hold active during deep sleep
    gpio_deep_sleep_hold_en(); 
    
    // Configure wake-up via the physical power switch
    esp_sleep_enable_ext0_wakeup((gpio_num_t)switchPwrPin, 0);
    
    delay(100);
    esp_deep_sleep_start();
}

void PowerManager::update() {
    Serial.println("[POWER] update");
    if (millis() - lastBatteryUpdate > batteryUpdateInterval) {
        batteryVoltage = readBatteryVoltage();
        batteryPercentage = calcBatteryPercentage(batteryVoltage);
        lastBatteryUpdate = millis();
    }
}

float PowerManager::readBatteryVoltage() {
    adcBuffer[bufferIndex] = analogReadMilliVolts(batteryPin);
    bufferIndex = (bufferIndex + 1) % SAMPLES;
    
    long sum = 0;
    for(int i = 0; i < SAMPLES; i++) sum += adcBuffer[i];
    
    float avgMv = (float)sum / SAMPLES;
    return (avgMv * 2.0f) / 1000.0f; // Adjust based on your voltage divider
}

int PowerManager::calcBatteryPercentage(float voltage) {
    if (voltage >= 4.15f) return 100;
    if (voltage <= 3.3f) return 0;
    
    long pct = map((long)(voltage * 100), 330, 415, 0, 100);
    return constrain(pct, 0, 100);
}

void PowerManager::setSystemLedState(bool on) {
    digitalWrite(systemLedPin, on ? HIGH : LOW);
}

bool PowerManager::isSwitchedOn() { 
    return digitalRead(switchPwrPin) == LOW; 
}