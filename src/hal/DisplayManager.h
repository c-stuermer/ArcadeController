/**
 * Project: Arcade Controller V0.2
 * File: DisplayManager.h
 * Description: Hardware abstraction for the TFT display using the high-speed TFT_eSPI library.
 */

#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h> 
#include "../config/Config.h"

class DisplayManager {
private:
    // The TFT_eSPI object is now instantiated directly (stack/member allocation).
    // Pins and chip config are automatically pulled from the library's User_Setup.h
    TFT_eSPI screen;

    uint8_t _currentBrightness = 100;
    int _currentBattery = 0;

public:
    DisplayManager();
    // Destructor removed: no dynamic memory allocation needs to be cleaned up anymore.

    // Initializes the display hardware
    void begin();
    
    // Clears the entire screen (fills with black)
    void clear(); 
    
    // Returns the raw TFT_eSPI pointer for advanced drawing operations
    TFT_eSPI* getGfx() { return &screen; }

    // --- UI Elements ---
    
    // Draws the top status bar including the title and battery indicator
    void drawHeader(const String& title);
    
    // Draws a progress bar at the bottom of the screen
    void drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color = 0xFFFF);
    
    // Clears the progress bar area
    void clearProgressBar();

    // --- State Setters & Getters ---
    
    // Sets the backlight brightness (0-100)
    void setBrightness(uint8_t level); 
    
    // Updates the internal battery state used by the UI elements
    void setBatteryLevel(int level);
    
    // Returns the current brightness level
    uint8_t getBrightness() const { return _currentBrightness; }
};