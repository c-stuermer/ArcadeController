/**
 * Project: Arcade Controller V1.0
 * File: apps/SpaceInvadersApp/SpaceInvadersEntities.h
 * Description: Game entities (Projectile, AlienEntity, PlayerEntity).
 *              Header-only for inlining in the hot game loop.
 */

#pragma once
#include <TFT_eSPI.h>
#include "SpaceInvadersSprites.h"

// --- PROJECTILE (bullet & bomb) ---
class Projectile {
public:
    int x, y;
    bool active = false;
    int speed;

    void fire(int startX, int startY, int s) {
        x = startX; y = startY; speed = s; active = true;
    }

    void update() {
        if (!active) return;
        y += speed;
        if (y < -10 || y > 140) active = false;
    }

    void draw(TFT_eSPI* gfx, uint16_t color) {
        if (active) gfx->fillRect(x, y, 2, 4, color);
    }
};

// --- ALIEN ---
class AlienEntity {
public:
    int x, y, type;
    bool active = true;

    void draw(TFT_eSPI* gfx, int animFrame) {
        if (!active) return;
        const unsigned char* sprite;
        if (type == 0) sprite = (animFrame == 0) ? alien_squid_1 : alien_squid_2;
        else if (type == 1) sprite = (animFrame == 0) ? alien_crab_1 : alien_crab_2;
        else sprite = (animFrame == 0) ? alien_octopus_1 : alien_octopus_2;

        gfx->drawBitmap(x, y, sprite, 16, 8, 0xF800);
    }

    bool checkHit(int px, int py) {
        if (!active) return false;
        // Hitbox: 16x8
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
    int x, y;

    void move(int dx, int screenWidth) {
        x += dx;
        if (x < 0) x = 0;
        if (x > screenWidth - 16) x = screenWidth - 16;
    }

    void draw(TFT_eSPI* gfx, uint16_t color) {
        gfx->drawBitmap(x, y, player_bmp, 16, 8, color);
    }
};
