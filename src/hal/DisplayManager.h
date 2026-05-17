/**
 * Project: Arcade Controller V1.2
 * File: DisplayManager.h
 * Description: Hardware abstraction for the TFT display using the high-speed TFT_eSPI library.
 */

#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../config/Config.h"
#include "../config/Colors.h"

class DisplayManager {
public:
    DisplayManager() = default;

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
    void drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color = Colors::WHITE);

    // Clears the progress bar area
    void clearProgressBar();

    // --- State Setters & Getters ---

    // Sets the backlight brightness (0-100)
    void setBrightness(uint8_t level);

    // Updates the internal battery state used by the UI elements
    void setBatteryLevel(int level);

    // Returns the current brightness level
    uint8_t getBrightness() const { return currentBrightness; }

private:
    // --- Screen geometry (160x128 landscape) ---
    static constexpr int SCREEN_W       = 160;
    static constexpr int HEADER_H       = 12;
    static constexpr int PROGRESS_Y     = 126;
    static constexpr int PROGRESS_H     = 2;

    // --- Battery indicator inside the header ---
    static constexpr int BATT_X         = 135;
    static constexpr int BATT_Y         = 2;
    static constexpr int BATT_W         = 20;
    static constexpr int BATT_H         = 8;
    static constexpr int BATT_LOW_PCT   = 20;   // <= 20% draws red instead of green

    // The TFT_eSPI object is instantiated directly (member, no dynamic alloc).
    // Pins and chip config are pulled from the library's User_Setup.h.
    TFT_eSPI screen;

    uint8_t currentBrightness = 100;
    int     currentBattery    = 0;
};
