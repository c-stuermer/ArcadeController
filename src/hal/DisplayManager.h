/**
 * Project: Arcade Controller V0.1
 * File: DisplayManager.h
 * Description: Wrapper for U8g2 library to handle OLED output.
 */

#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "../config/Config.h"

class DisplayManager {
private:
    // U8g2 instance for SH1106 I2C 128x64 Display
    U8G2_SH1106_128X64_NONAME_F_HW_I2C gfx;

public:
    DisplayManager();

    void init();
    void clear(); 
    void show(); 
    
    // Access to raw U8g2 object if needed
    U8G2* getGfx() { return &gfx; }

    // --- Header Variants ---

    // 1. Full System Header (Title + Battery Icon + Line)
    void drawHeader(const String& title, int batteryPercent, bool isUsbConnected);

    // 2. Minimal Header (Centered Title + Line)
    void drawHeader(const String& title); 
};