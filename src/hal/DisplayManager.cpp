/**
 * Project: Arcade Controller V0.1
 * File: DisplayManager.cpp
 * Description: Implementation of OLED display logic.
 */

#include "DisplayManager.h"

// CRITICAL: 'gfx' must be initialized in the initializer list
// to properly set rotation (U8G2_R0) and reset pin (U8X8_PIN_NONE)
// BEFORE the constructor body executes.
DisplayManager::DisplayManager() : gfx(U8G2_R0, U8X8_PIN_NONE) {
    // Constructor body remains empty
}

void DisplayManager::init() {
    // 1. Pre-Check I2C Bus
    Wire.begin(); // Starts I2C (SDA, SCL)
    
    Wire.beginTransmission(0x3C); // 0x3C is standard for SH1106/SSD1306
    byte error = Wire.endTransmission();

    if (error == 0) {
        Serial.println("[I2C] Display found at address 0x3C");
    } else {
        Serial.print("[I2C] ERROR: No Display found! (Error code: ");
        Serial.print(error);
        Serial.println(")");
        // Error codes: 2=NACK (Not found), 4=Other
    }
    
    gfx.begin();
}

void DisplayManager::clear() {
    gfx.clearBuffer();
}

void DisplayManager::show() {
    gfx.sendBuffer();
}

// --- Header Logic ---

// VARIANT 1: Full System Header
void DisplayManager::drawHeader(const String& title, int batteryPercent, bool isUsbConnected) {
    // Bottom line (Y=12)
    gfx.drawHLine(0, 12, 128); 

    // Title (Left aligned)
    gfx.setFont(u8g2_font_6x10_tf); 
    gfx.setCursor(2, 9);
    gfx.print(title);

    // Battery Icon (Top Right)
    int battX = 110; int battY = 2; int battW = 14; int battH = 8;
    
    // Battery Body
    gfx.drawFrame(battX, battY, battW, battH);
    gfx.drawBox(battX + battW, battY + 2, 2, 4); // Nipple

    // Content
    if (isUsbConnected) {
        // Charging Mode
        gfx.drawBox(battX + 2, battY + 2, battW - 4, battH - 4);
        // Small indicator dot next to it
        gfx.drawDisc(battX - 3, battY + 4, 1); 
    } else {
        // Percentage Mode
        int innerWidth = battW - 2;
        int fillWidth = (innerWidth * batteryPercent) / 100;
        if (fillWidth > 0) gfx.drawBox(battX + 1, battY + 1, fillWidth, battH - 2);
    }
}

// VARIANT 2: Minimal Header
void DisplayManager::drawHeader(const String& title) {
    // Bottom line
    gfx.drawHLine(0, 12, 128); 

    // Title (Centered)
    gfx.setFont(u8g2_font_6x10_tf); 
    
    int w = gfx.getStrWidth(title.c_str());
    int x = (128 - w) / 2; // Centering formula
    
    gfx.setCursor(x, 9);
    gfx.print(title);
}