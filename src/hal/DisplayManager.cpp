/**
 * Project: Arcade Controller V0.2
 * File: DisplayManager.cpp
 * Description: Implementation of display routines and UI elements.
 */

#include "DisplayManager.h"

DisplayManager::DisplayManager() {
    // Constructor is empty now. 
    // The 'screen' object is automatically initialized when DisplayManager is created.
}

void DisplayManager::begin() {
    Serial.println("[DISPLAY] Init TFT_eSPI...");
    
    // TFT_eSPI initialization
    screen.begin(); 
    
    // Orientation: 3 = Landscape
    screen.setRotation(3); 
    
    // Set initial text color (White) and size
    screen.setTextSize(1);
    screen.setTextColor(0xFFFF); 
}

void DisplayManager::clear() {
    // Fill screen with black
    screen.fillScreen(0x0000); 
}

void DisplayManager::drawHeader(const String& title) {
    screen.setTextColor(0xFFFF, 0x0000); 
    screen.setTextSize(1);

    screen.setCursor(2, 2);
    screen.print(title);
    
    screen.drawFastHLine(0, 12, 160, 0xFFFF); 

    int bx = 135; 
    int by = 2;
    int bw = 20;
    int bh = 8;

    // Draw battery outline
    screen.drawRect(bx, by, bw, bh, 0xFFFF); 
    screen.drawFastVLine(bx + bw, by + 2, 4, 0xFFFF); 

    int maxWidth = bw - 2;
    int fillWidth = map(_currentBattery, 0, 100, 0, maxWidth);
    
    // Red color if battery is critical (<= 20%), green otherwise
    uint16_t color = (_currentBattery > 20) ? 0x07E0 : 0xF800;

    // Draw filled battery level
    if (fillWidth > 0) {
        screen.fillRect(bx + 1, by + 1, fillWidth, bh - 2, color);
    }

    // Clear the remaining empty space inside the battery
    if (fillWidth < maxWidth) {
        screen.fillRect(bx + 1 + fillWidth, by + 1, maxWidth - fillWidth, bh - 2, 0x0000);
    }
}

void DisplayManager::setBrightness(uint8_t level) {
    if (level > 100) level = 100;
    
    // Store current brightness level for later reference
    _currentBrightness = level; 
    
    // Map 0-100% to 0-255 for the PWM signal
    int dutyCycle = map(level, 0, 100, 0, 255);
    analogWrite(PinConfig::DISP_BLK, dutyCycle);
}

void DisplayManager::setBatteryLevel(int level) {
    _currentBattery = level; 
}

void DisplayManager::drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color) {
    if (maxVal == 0) return;
    if (current > maxVal) current = maxVal;
    
    // Calculate bar width (max 160 pixels wide)
    int barWidth = (current * 160) / maxVal;
    
    // Draw only if width is greater than 0
    if (barWidth > 0) {
        screen.fillRect(0, 126, barWidth, 2, color);
    }
}

void DisplayManager::clearProgressBar() {
    // Overwrite the entire bottom progress bar area with black
    screen.fillRect(0, 126, 160, 2, 0x0000);
}