/**
 * Project: Arcade Controller V1.3
 * File: DisplayManager.cpp
 * Description: Implementation of display routines and UI elements.
 */

#include "DisplayManager.h"

void DisplayManager::begin() {
    Serial.println("[DISPLAY] Init TFT_eSPI...");

    screen.begin();
    screen.setRotation(3);   // Landscape

    // Allocate the full-screen off-screen buffer.
    // All subsequent draw calls target this sprite; flush() commits it.
    buffer.createSprite(SCREEN_W, SCREEN_H);
    buffer.setTextWrap(false);
    buffer.setTextSize(1);
    buffer.setTextColor(Colors::WHITE);

    Serial.println("[DISPLAY] Render buffer allocated (40KB)");
}

void DisplayManager::clear() {
    buffer.fillSprite(Colors::BLACK);
}

void DisplayManager::drawHeader(const String& title) {
    buffer.setTextColor(Colors::WHITE, Colors::BLACK);
    buffer.setTextSize(1);

    buffer.setCursor(2, 2);
    buffer.print(title);

    // Horizontal separator line just below the title
    buffer.drawFastHLine(0, HEADER_H, SCREEN_W, Colors::WHITE);

    // --- Battery indicator (outline + cap + fill) ---
    buffer.drawRect(BATT_X, BATT_Y, BATT_W, BATT_H, Colors::WHITE);
    buffer.drawFastVLine(BATT_X + BATT_W, BATT_Y + 2, 4, Colors::WHITE);

    const int maxWidth  = BATT_W - 2;
    const int fillWidth = map(currentBattery, 0, 100, 0, maxWidth);

    const uint16_t fillColor = (currentBattery > BATT_LOW_PCT) ? Colors::GREEN : Colors::RED;

    if (fillWidth > 0) {
        buffer.fillRect(BATT_X + 1, BATT_Y + 1, fillWidth, BATT_H - 2, fillColor);
    }
    if (fillWidth < maxWidth) {
        buffer.fillRect(BATT_X + 1 + fillWidth, BATT_Y + 1, maxWidth - fillWidth, BATT_H - 2, Colors::BLACK);
    }
}

void DisplayManager::flush() {
    buffer.pushSprite(0, 0);
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
        buffer.fillRect(0, PROGRESS_Y, barWidth, PROGRESS_H, color);
    }
}

void DisplayManager::clearProgressBar() {
    buffer.fillRect(0, PROGRESS_Y, SCREEN_W, PROGRESS_H, Colors::BLACK);
}

// --- Primitive drawing -----------------------------------------------------

void DisplayManager::drawText(int x, int y, const String& text, uint16_t color, uint8_t size) {
    buffer.setTextSize(size);
    buffer.setTextColor(color);
    buffer.setCursor(x, y);
    buffer.print(text);
}

void DisplayManager::fillRect(int x, int y, int w, int h, uint16_t color) {
    buffer.fillRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int x, int y, int r, uint16_t color) {
    buffer.drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int x, int y, int r, uint16_t color) {
    buffer.fillCircle(x, y, r, color);
}

void DisplayManager::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    buffer.drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h, uint16_t color) {
    buffer.drawBitmap(x, y, bitmap, w, h, color);
}

void DisplayManager::drawRect(int x, int y, int w, int h, uint16_t color) {
    buffer.drawRect(x, y, w, h, color);
}
