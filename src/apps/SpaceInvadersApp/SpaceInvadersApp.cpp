/**
 * Project: Arcade Controller V1.1
 * File: apps/SpaceInvadersApp/SpaceInvadersApp.cpp
 */

#include "SpaceInvadersApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/InputHandler.h"
#include "../../hal/SoundManager.h"
#include "SpaceInvadersSprites.h"

void SpaceInvadersApp::start() {
    Serial.println("[APP] SpaceInvadersApp starting...");
    auto* gfx = system->getDisplay()->getGfx();
    gfx->fillScreen(0x0000); // Clear the screen once at start
    currentState = GameState::TITLE;
    drawTitleScreen();
}

void SpaceInvadersApp::stop() {
    system->getDisplay()->clearProgressBar(); // Clear the progress bar on exit
}

void SpaceInvadersApp::initGame() {
    auto* gfx = system->getDisplay()->getGfx();
    gfx->fillScreen(0x0000);

    // Player setup
    player.x = gfx->width() / 2 - 8;
    player.y = gfx->height() - 15;

    // Alien setup (3 rows of 5)
    for (int i = 0; i < NUM_ALIENS; i++) {
        aliens[i].x = 20 + (i % 5) * 25;
        aliens[i].y = 25 + (i / 5) * 15;
        aliens[i].active = true;
        aliens[i].type = i / 5;
    }

    // Reset state variables
    score = 0;
    bullet.active = false;
    for (auto& b : bombs) b.active = false;
    ufoActive = false;
    nextUfoTime = millis() + random(10000, 20000);
    lastAlienMove = millis();
    lastFrameTime = millis();

    currentState = GameState::PLAYING;
}

void SpaceInvadersApp::update() {
    auto input = system->getInput();
    auto* gfx = system->getDisplay()->getGfx();

    // --- EXIT COMBO: SELECT + L2 + R2 (2s hold) ---
    // Works from any state (TITLE, PLAYING, PAUSED, GAMEOVER).
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);

    if (durSel > 0 && durL2 > 0 && durR2 > 0) {
        unsigned long comboTime = min(durSel, min(durL2, durR2));
        system->getDisplay()->drawProgressBar(comboTime, 2000, 0xF800);

        if (comboTime >= 2000) {
            system->getDisplay()->clearProgressBar();
            system->getAppManager()->startApp(AppId::Menu);
            return;
        }
    } else {
        system->getDisplay()->clearProgressBar();
    }

    // 1. TITLE ANIMATION
    if (currentState == GameState::TITLE) {
        updateTitleAnimation(); // Extracted for clarity
        return;
    }

    if (currentState != GameState::PLAYING) return;
    if (millis() - lastFrameTime < 30) return;
    lastFrameTime = millis();

    // 2. PLAYER & BULLET
    player.draw(gfx, 0x0000); // Erase
    if (moveLeft) player.move(-3, gfx->width());
    if (moveRight) player.move(3, gfx->width());
    player.draw(gfx, 0x07E0); // Draw

    bullet.draw(gfx, 0x0000);
    bullet.update();
    bullet.draw(gfx, 0xFFFF);

    // 3. ALIEN BOMBS
    for (auto& b : bombs) {
        b.draw(gfx, 0x0000);
        b.update();
        if (b.active) {
            b.draw(gfx, 0xFBE0);
            // Check collision with player
            if (b.x > player.x && b.x < player.x + 16 && b.y > player.y && b.y < player.y + 8) {
                currentState = GameState::GAMEOVER;
                system->getSound()->play(SoundEffect::EXPLOSION);
                drawGameOverScreen();
            }
        }
    }

    // 4. ALIEN MOVEMENT & ANIMATION
    if (millis() - lastAlienMove > 500) {
        lastAlienMove = millis();
        animationFrame = !animationFrame;
        gfx->fillScreen(0x0000);

        bool hitEdge = false;
        for (auto& a : aliens) {
            if (!a.active) continue;
            a.x += (alienDirection * 5);
            if (a.x <= 5 || a.x >= gfx->width() - 20) hitEdge = true;
        }

        if (hitEdge) {
            alienDirection *= -1;
            for (auto& a : aliens) if (a.active) a.y += 8;
        }

        // Drop a bomb (random)
        if (random(10) > 7) {
            int shooter = random(NUM_ALIENS);
            if (aliens[shooter].active) {
                for (auto& b : bombs) if (!b.active) { b.fire(aliens[shooter].x + 7, aliens[shooter].y + 8, 3); break; }
            }
        }
    }

    // 5. COLLISION CHECK
    if (bullet.active) {
        for (auto& a : aliens) {
            if (a.checkHit(bullet.x, bullet.y)) {
                bullet.active = false;
                score++;
                system->getSound()->play(SoundEffect::EXPLOSION);
                if (score >= NUM_ALIENS) initGame();
                break;
            }
        }
    }

    // 6. REDRAW
    drawAliens();
}

void SpaceInvadersApp::onInput(ControlEvent ev, EventType type) {
    bool isPressed = (type == EventType::PRESSED);

    // Track joystick movement globally
    if (ev == ControlEvent::JOY_LEFT) moveLeft = isPressed;
    if (ev == ControlEvent::JOY_RIGHT) moveRight = isPressed;

    if (!isPressed) return;

    if (currentState == GameState::TITLE && ev == ControlEvent::BTN_START) {
        initGame();
    }
    else if (currentState == GameState::PLAYING) {
        if (ev == ControlEvent::BTN_START) {
            currentState = GameState::PAUSED;
            drawPauseScreen();
        } else if (ev == ControlEvent::BTN_A && !bullet.active) {
            bullet.fire(player.x + 7, player.y, -5); // -5 = upward speed
            system->getSound()->play(SoundEffect::LASER);
        }
    }
    else if (currentState == GameState::PAUSED && ev == ControlEvent::BTN_START) {
        system->getDisplay()->getGfx()->fillScreen(0x0000);
        currentState = GameState::PLAYING;
    }
    else if (currentState == GameState::GAMEOVER && ev == ControlEvent::BTN_START) {
        system->getDisplay()->getGfx()->fillScreen(0x0000);
        currentState = GameState::TITLE;
        drawTitleScreen();
    }
}

// --- UI Screens ---

void SpaceInvadersApp::drawTitleScreen() {
    auto* gfx = system->getDisplay()->getGfx();
    gfx->fillScreen(0x0000);

    gfx->setTextSize(2);
    gfx->setTextColor(0x07E0); // Green
    gfx->drawString("SPACE", 50, 10);
    gfx->drawString("INVADERS", 32, 30);

    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    gfx->drawString("V1.1 ARCADE EDITION", 25, 115);
}

void SpaceInvadersApp::updateTitleAnimation() {
    auto* gfx = system->getDisplay()->getGfx();
    static unsigned long lastTitleAnim = 0;
    static int titleX = 20; // Start a bit further left for the larger aliens
    static int titleDir = 1;
    static bool titleFrame = false;

    if (millis() - lastTitleAnim > 100) {
        lastTitleAnim = millis();

        // Clear the area used by the large aliens (32 px tall instead of 20)
        gfx->fillRect(0, 60, 160, 35, 0x0000);

        titleX += (titleDir * 2);
        // Adjust the edge check to the larger overall width (~110 px)
        if (titleX <= 5 || titleX >= 160 - 110) titleDir *= -1;
        titleFrame = !titleFrame;

        for (int i = 0; i < 3; i++) {
            int xPos = titleX + (i * 40); // More spacing between the larger aliens
            const unsigned char* sprite;

            if (i == 0) sprite = titleFrame ? alien_squid_1 : alien_squid_2;
            else if (i == 1) sprite = titleFrame ? alien_crab_1 : alien_crab_2;
            else sprite = titleFrame ? alien_octopus_1 : alien_octopus_2;

            // --- Manual 2x scaling ---
            int scale = 2;
            for (int y = 0; y < 8; y++) {
                uint16_t rowData = (pgm_read_byte(&sprite[y * 2]) << 8) | pgm_read_byte(&sprite[y * 2 + 1]);
                for (int x = 0; x < 16; x++) {
                    if (rowData & (1 << (15 - x))) {
                        gfx->fillRect(xPos + (x * scale), 65 + (y * scale), scale, scale, 0xF800);
                    }
                }
            }
        }
    }
}

void SpaceInvadersApp::drawPauseScreen() {
    auto* gfx = system->getDisplay()->getGfx();
    // Draw a box over the game instead of clearing the whole screen
    gfx->fillRect(30, 40, 100, 40, 0x0000);
    gfx->drawRect(30, 40, 100, 40, 0xFFFF);

    gfx->setTextSize(2);
    gfx->setTextColor(0xFFE0); // Yellow
    gfx->drawString("PAUSED", 44, 52);
}

void SpaceInvadersApp::drawGameOverScreen() {
    auto* gfx = system->getDisplay()->getGfx();
    gfx->fillScreen(0x0000);

    gfx->setTextSize(2);
    gfx->setTextColor(0xF800); // Red
    gfx->drawString("GAME OVER", 26, 30);

    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    gfx->drawString("Score: " + String(score), 55, 60);
    gfx->drawString("PRESS START", 45, 100);
}

void SpaceInvadersApp::drawAliens() {
    auto* gfx = system->getDisplay()->getGfx();
    for (auto& a : aliens) a.draw(gfx, animationFrame);
}
