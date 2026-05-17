/**
 * Project: Arcade Controller V1.2
 * File: hal/PowerManager.cpp
 * Description: Power management and ghost-glow prevention during deep sleep.
 *
 * Carry-over of V0.2 with two minor cleanups:
 *  - Removed per-loop "[POWER] update" debug print.
 *  - All reset-button related code is gone (Reset is on the ESP32 EN-pin).
 */

#include "PowerManager.h"
#include "driver/rtc_io.h" // REQUIRED for gpio_hold functions

PowerManager::PowerManager() {
    for (int i = 0; i < SAMPLES; i++) adcBuffer[i] = 0;
}

void PowerManager::begin() {
    Serial.println("[POWER] Begin");

    // 1. Release pin holds set during the last deep sleep.
    gpio_hold_dis((gpio_num_t)backlightPin);
    gpio_hold_dis((gpio_num_t)displayRstPin);

    // 2. Configure standard pins
    pinMode(systemLedPin, OUTPUT);
    pinMode(switchPwrPin, INPUT_PULLUP);
    pinMode(batteryPin, INPUT);

    // Take over display pins for power management
    pinMode(backlightPin, OUTPUT);
    pinMode(displayRstPin, OUTPUT);

    // 3. Initial states
    setSystemLedState(true);

    // 4. Initial battery read (fill the smoothing buffer)
    for (int i = 0; i < SAMPLES; i++) {
        adcBuffer[i] = analogReadMilliVolts(batteryPin);
        delay(2);
    }
    batteryVoltage    = readBatteryVoltage();
    batteryPercentage = calcBatteryPercentage(batteryVoltage);
}

void PowerManager::update() {
    if (millis() - lastBatteryUpdate > BATTERY_UPDATE_MS) {
        batteryVoltage    = readBatteryVoltage();
        batteryPercentage = calcBatteryPercentage(batteryVoltage);
        lastBatteryUpdate = millis();
    }
}

bool PowerManager::isSwitchedOn() {
    return digitalRead(switchPwrPin) == LOW;
}

void PowerManager::setSystemLedState(bool on) {
    digitalWrite(systemLedPin, on ? HIGH : LOW);
}

void PowerManager::enterDeepSleep() {
    Serial.println("[POWER] ENTERING DEEP SLEEP");
    Serial.flush();

    setSystemLedState(false);

    // Shut down peripherals and activate GPIO hold
    turnOffPeripherals();

    // Keep the hold active during deep sleep
    gpio_deep_sleep_hold_en();

    // Wake up via the physical power switch
    esp_sleep_enable_ext0_wakeup((gpio_num_t)switchPwrPin, 0);

    delay(100);
    esp_deep_sleep_start();
}

// --- Private ----------------------------------------------------------------

void PowerManager::turnOffPeripherals() {
    // 1. Turn off backlight
    gpio_reset_pin((gpio_num_t)backlightPin);
    pinMode(backlightPin, OUTPUT);
    digitalWrite(backlightPin, LOW);

    // 2. Hard-disable the display controller
    digitalWrite(displayRstPin, LOW);

    // 3. Make I2C pins high-impedance
    pinMode(PinConfig::I2C_SDA, INPUT);
    pinMode(PinConfig::I2C_SCL, INPUT);

    // 4. Make SPI pins high-impedance
    pinMode(PinConfig::SPI_MOSI, INPUT);
    pinMode(PinConfig::SPI_SCLK, INPUT);
    pinMode(PinConfig::DISP_CS, INPUT);
    pinMode(PinConfig::DISP_DC, INPUT);

    // 5. "GHOST GLOW" FIX: Freeze the current pin states
    gpio_hold_en((gpio_num_t)backlightPin);
    gpio_hold_en((gpio_num_t)displayRstPin);

    // Built-in LED off and held
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    gpio_hold_en((gpio_num_t)LED_BUILTIN);
}

float PowerManager::readBatteryVoltage() {
    adcBuffer[bufferIndex] = analogReadMilliVolts(batteryPin);
    bufferIndex = (bufferIndex + 1) % SAMPLES;

    long sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adcBuffer[i];

    const float avgMv = (float)sum / SAMPLES;
    return (avgMv * VDIV_RATIO) / 1000.0f;   // mV -> V, undoing the voltage divider
}

int PowerManager::calcBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_FULL_V)  return 100;
    if (voltage <= BATTERY_EMPTY_V) return 0;

    // Linear interpolation between empty and full. map() works on longs, so
    // we scale the voltages by 100 to keep two decimals of resolution.
    const long pct = map((long)(voltage * 100),
                         (long)(BATTERY_EMPTY_V * 100),
                         (long)(BATTERY_FULL_V  * 100),
                         0, 100);
    return constrain(pct, 0, 100);
}
