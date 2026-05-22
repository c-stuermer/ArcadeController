/**
 * Project: Arcade Controller V1.3
 * File: IDisplay.h
 * Description: Abstract interface for the display subsystem. Apps depend
 *              on this contract, never on the concrete DisplayManager,
 *              and never on the underlying TFT_eSPI driver. The interface
 *              intentionally does not expose getGfx() — the rendering
 *              backend stays an implementation detail behind drawText()
 *              and fillRect().
 */

#pragma once
#include <Arduino.h>
#include "../../config/Colors.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void begin()                                                                                = 0;
    virtual void clear()                                                                                = 0;

    // --- High-level UI elements (kept from V1.2) ---
    virtual void drawHeader(const String& title)                                                        = 0;
    virtual void drawProgressBar(unsigned long current, unsigned long maxVal, uint16_t color = Colors::WHITE) = 0;
    virtual void clearProgressBar()                                                                     = 0;

    // --- Primitive drawing (V1.3) ---
    // Generic primitives that replace the previous getGfx() leak. Apps
    // compose menus, lists, overlays and diagnostic UIs out of these
    // calls without ever touching the rendering backend.
    virtual void drawText(int x, int y, const String& text, uint16_t color, uint8_t size = 1)           = 0;
    virtual void fillRect(int x, int y, int w, int h, uint16_t color)                                   = 0;
    virtual void drawCircle(int x, int y, int r, uint16_t color)                                        = 0;
    virtual void fillCircle(int x, int y, int r, uint16_t color)                                        = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1, uint16_t color)                               = 0;

    // --- State setters & getters ---
    virtual void setBrightness(uint8_t level)                                                           = 0;
    virtual void setBatteryLevel(int level)                                                             = 0;
    virtual uint8_t getBrightness() const                                                               = 0;
};
