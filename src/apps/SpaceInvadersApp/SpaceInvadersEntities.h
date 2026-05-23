/**
 * Project: Arcade Controller V1.3
 * File: apps/SpaceInvadersApp/SpaceInvadersEntities.h
 * Description: Game entities (Projectile, AlienEntity, PlayerEntity).
 *              Header-only for inlining in the hot game loop.
 *              All drawing is done through IDisplay so entities never
 *              depend on the concrete rendering backend.
 */

#pragma once
#include <Arduino.h>
#include "../../hal/interfaces/IDisplay.h"
#include "../../config/Colors.h"
#include "SpaceInvadersSprites.h"

// --- PROJECTILE (bullet & bomb) ---
class Projectile {
public:
    int  x = 0, y = 0;
    bool active = false;
    int  speed  = 0;

    void fire(int startX, int startY, int s) {
        x = startX; y = startY; speed = s; active = true;
    }

    void update() {
        if (!active) return;
        y += speed;
        if (y < -10 || y > 140) active = false;
    }

    void draw(IDisplay* display, uint16_t color) {
        if (active) display->fillRect(x, y, 2, 4, color);
    }
};

// --- ALIEN ---
class AlienEntity {
public:
    int  x = 0, y = 0, type = 0;
    bool active = true;

    void draw(IDisplay* display, int animFrame) {
        if (!active) return;
        const unsigned char* sprite;
        if      (type == 0) sprite = (animFrame == 0) ? alien_squid_1   : alien_squid_2;
        else if (type == 1) sprite = (animFrame == 0) ? alien_crab_1    : alien_crab_2;
        else                sprite = (animFrame == 0) ? alien_octopus_1 : alien_octopus_2;
        display->drawBitmap(x, y, sprite, 16, 8, Colors::RED);
    }

    bool checkHit(int px, int py) {
        if (!active) return false;
        if (px >= x && px <= x + 16 && py >= y && py <= y + 8) {
            active = false;
            return true;
        }
        return false;
    }
};

// --- PLAYER ---
class PlayerEntity {
public:
    int x = 72, y = 113;

    void move(int dx, int maxX) {
        x += dx;
        if (x < 0)         x = 0;
        if (x > maxX - 16) x = maxX - 16;
    }

    void draw(IDisplay* display, uint16_t color) {
        display->drawBitmap(x, y, player_bmp, 16, 8, color);
    }
};
