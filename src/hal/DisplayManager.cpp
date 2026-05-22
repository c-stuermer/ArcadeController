/**
 * Project: Arcade Controller V1.3
 * File: DisplayManager.cpp
 * Description: Implementation of display routines and UI elements.
 */

#include "DisplayManager.h"

void DisplayManager::begin() {
    Serial.println("[DISPLAY] Init TFT_eSPI...");

    screen.begin();

    // Orientation: 3 = Landscape
    screen.setRotation(3);

    // Set initial text color (White) and size
    screen.setTextSize(1);
    screen.setTextColor(Colors::WHITE);
}

void DisplayManager::clear() {
    screen.fillScreen(Colors::BLACK);
}

void DisplayManager::drawHeader(const String& title) {
    screen.setTextColor(Colors::WHITE, Colors::BLACK);
    screen.setTextSize(1);

    screen.setCursor(2, 2);
    screen.print(title);

    // Horizontal separator line just below the title
    screen.drawFastHLine(0, HEADER_H, SCREEN_W, Colors::WHITE);

    // --- Battery indicator (outline + cap + fill) ---
    screen.drawRect(BATT_X, BATT_Y, BATT_W, BATT_H, Colors::WHITE);
    screen.drawFastVLine(BATT_X + BATT_W, BATT_Y + 2, 4, Colors::WHITE);

    const int maxWidth  = BATT_W - 2;
    const int fillWidth = map(currentBattery, 0, 100, 0, maxWidth);

    const uint16_t fillColor = (currentBattery > BATT_LOW_PCT) ? Colors::GREEN : Colors::RED;

    // Filled portion (current charge)
    if (fillWidth > 0) {
        screen.fillRect(BATT_X + 1, BATT_Y + 1, fillWidth, BATT_H - 2, fillColor);
    }
    // Remaining empty portion (clear so old levels don't bleed through)
    if (fillWidth < maxWidth) {
        screen.fillRect(BATT_X + 1 + fillWidth, BATT_Y + 1, maxWidth - fillWidth, BATT_H - 2, Colors::BLACK);
    }
}

void DisplayManager::setBrightness(uint8_t level) {
    if (level > 100) level = 100;
    currentBrightness = level;

    // Map 0-100% to 0-255 for the PWM signal
    const int dutyCycle = map(level, 0, 100, 0, 255);
    analogWrite(PinConfig::DISP_BLK, dutyCycle);
}

void DisplayManager::setBatteryLevel(int level) {
    currentBattery = level;
}

void DisplayManager::drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color) {
    if (maxVal == 0) return;
    if (current > maxVal) current = maxVal;

    const int barWidth = (current * SCREEN_W) / maxVal;

    if (barWidth > 0) {
        screen.fillRect(0, PROGRESS_Y, barWidth, PROGRESS_H, color);
    }
}

void DisplayManager::clearProgressBar() {
    screen.fillRect(0, PROGRESS_Y, SCREEN_W, PROGRESS_H, Colors::BLACK);
}

// --- Primitive drawing (V1.3) ----------------------------------------------

void DisplayManager::drawText(int x, int y, const String& text, uint16_t color, uint8_t size) {
    screen.setTextSize(size);
    screen.setTextColor(color);
    screen.setCursor(x, y);
    screen.print(text);
}

void DisplayManager::fillRect(int x, int y, int w, int h, uint16_t color) {
    screen.fillRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int x, int y, int r, uint16_t color) {
    screen.drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int x, int y, int r, uint16_t color) {
    screen.fillCircle(x, y, r, color);
}

void DisplayManager::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    screen.drawLine(x0, y0, x1, y1, color);
}
