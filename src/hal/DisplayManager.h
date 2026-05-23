/**
 * Project: Arcade Controller V1.3
 * File: DisplayManager.h
 * Description: Hardware abstraction for the TFT display using the high-speed TFT_eSPI library.
 *              Implements the IDisplay interface so apps can depend on the
 *              abstract contract rather than this concrete class.
 */

#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../config/Config.h"
#include "../config/Colors.h"
#include "interfaces/IDisplay.h"

// TFT_eSprite is TFT_eSPI's off-screen RAM buffer. All draw calls target the
// sprite; flush() pushes the completed frame to the physical display in one
// SPI transfer, eliminating per-primitive flicker.

class DisplayManager : public IDisplay {
public:
    DisplayManager() = default;

    // Initializes the display hardware
    void begin() override;

    // Clears the entire screen (fills with black)
    void clear() override;

    // Returns the raw TFT_eSPI pointer for advanced drawing operations.
    // Intentionally NOT part of the IDisplay interface (V1.3): apps that
    // depend only on IDisplay cannot reach the rendering backend. Other
    // apps that still hold a concrete DisplayManager* (via ISystem)
    // continue to use this until they are ported to the interface.
    TFT_eSPI* getGfx() { return &screen; }

    // --- UI Elements ---

    // Draws the top status bar including the title and battery indicator
    void drawHeader(const String& title) override;

    // Draws a progress bar at the bottom of the screen
    void drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color = Colors::WHITE) override;

    // Clears the progress bar area
    void clearProgressBar() override;

    // --- Primitive drawing (V1.3) ---

    // Draws a single text run at (x, y) in the given color and text size.
    // Background is left untouched - clear the area first if needed.
    void drawText(int x, int y, const String& text, uint16_t color, uint8_t size = 1) override;

    // Fills an axis-aligned rectangle with a solid color.
    void fillRect(int x, int y, int w, int h, uint16_t color) override;

    // Circle outline / filled circle.
    void drawCircle(int x, int y, int r, uint16_t color) override;
    void fillCircle(int x, int y, int r, uint16_t color) override;

    // Straight line between two points.
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color) override;

    // 1-bit PROGMEM bitmap (transparent background).
    void drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h, uint16_t color) override;

    // Rectangle outline (unfilled).
    void drawRect(int x, int y, int w, int h, uint16_t color) override;

    // --- State Setters & Getters ---

    // Sets the backlight brightness (0-100)
    void setBrightness(uint8_t level) override;

    // Updates the internal battery state used by the UI elements
    void setBatteryLevel(int level) override;

    // Returns the current brightness level
    uint8_t getBrightness() const override { return currentBrightness; }

    // Pushes the completed off-screen frame to the physical display
    void flush() override;

private:
    // --- Screen geometry (160x128 landscape) ---
    static constexpr int SCREEN_W       = 160;
    static constexpr int SCREEN_H       = 128;
    static constexpr int HEADER_H       = 12;
    static constexpr int PROGRESS_Y     = 126;
    static constexpr int PROGRESS_H     = 2;

    // --- Battery indicator inside the header ---
    static constexpr int BATT_X         = 135;
    static constexpr int BATT_Y         = 2;
    static constexpr int BATT_W         = 20;
    static constexpr int BATT_H         = 8;
    static constexpr int BATT_LOW_PCT   = 20;   // <= 20% draws red instead of green

    // The TFT_eSPI object drives the physical display over SPI.
    // Pins and chip config are pulled from the library's User_Setup.h.
    TFT_eSPI   screen;

    // Full-screen off-screen buffer (160×128, 16-bit colour = 40 KB).
    // Initialised in begin() via createSprite(). All draw calls target
    // this sprite; flush() commits it to the screen with pushSprite().
    TFT_eSprite buffer { &screen };

    uint8_t currentBrightness = 100;
    int     currentBattery    = 0;
};
