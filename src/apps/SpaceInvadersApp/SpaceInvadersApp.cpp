/**
 * Project: Arcade Controller V1.3
 * File: apps/SpaceInvadersApp/SpaceInvadersApp.cpp
 * Description: Space Invaders game logic and rendering.
 *
 *              Rendering strategy (V1.3):
 *                Every PLAYING frame calls display->clear() once, draws all
 *                entities, then display->flush(). The TFT_eSprite buffer
 *                absorbs all intermediate draw calls in RAM; the physical
 *                SPI transfer only happens on flush(), giving zero flicker
 *                regardless of how many primitives are drawn per frame.
 *
 *              V1.3+ additions:
 *                - 3 lives: bomb hit -> DYING overlay (1.5 s) -> respawn
 *                - Level progression: nextWave() on wave clear;
 *                  alienMoveInterval -50 ms/wave (floor 100 ms)
 *                - Classic speed curve: effectiveInterval =
 *                  max(50, alienMoveInterval * remaining / NUM_ALIENS)
 *                - NVS high-score (Preferences, namespace "invaders")
 *                - UFO: random(1,7)*50 pts (50–300)
 *                - HUD: drawHud() — score / hi / level + life pips
 */

#include "SpaceInvadersApp.h"
#include "../../config/Colors.h"
#include "SpaceInvadersSprites.h"
#include <Preferences.h>

// Screen constants (landscape 160x128)
static constexpr int GAME_W = 160;
static constexpr int GAME_H = 128;
static constexpr int UFO_Y  = 18;   // Y-position of the UFO row

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------

SpaceInvadersApp::SpaceInvadersApp(IDisplay*      display,
                                   ISound*        sound,
                                   IInputH*       input,
                                   IAppNavigator* appNavigator)
    : App(),
      display(display),
      sound(sound),
      input(input),
      appNavigator(appNavigator) {}

// -------------------------------------------------------------------------
// Lifecycle
// -------------------------------------------------------------------------

void SpaceInvadersApp::start() {
    Serial.println("[APP] SpaceInvadersApp starting...");
    moveLeft = moveRight = false;

    // Load persisted high-score from NVS
    Preferences prefs;
    prefs.begin("invaders", true);   // read-only
    highScore = prefs.getInt("hiscore", 0);
    prefs.end();

    currentState = GameState::TITLE;
    drawTitleScreen();
}

void SpaceInvadersApp::stop() {
    display->clearProgressBar();
}

// -------------------------------------------------------------------------
// Game initialisation (full reset: score, lives, level 1)
// -------------------------------------------------------------------------

void SpaceInvadersApp::initGame() {
    player.x = GAME_W / 2 - 8;
    player.y = GAME_H - 15;

    // 3 rows of 5 aliens
    for (int i = 0; i < NUM_ALIENS; i++) {
        aliens[i].x      = 20 + (i % 5) * 25;
        aliens[i].y      = 25 + (i / 5) * 15;
        aliens[i].active = true;
        aliens[i].type   = i / 5;   // row 0=squid, 1=crab, 2=octopus
    }

    score              = 0;
    lives              = 3;
    level              = 1;
    alienDirection     = 1;
    animationFrame     = 0;
    alienMoveInterval  = 500;
    bullet.active      = false;
    for (auto& b : bombs) b.active = false;

    ufoActive     = false;
    nextUfoTime   = millis() + random(10000, 20000);
    lastAlienMove = millis();
    lastAlienAnim = millis();
    lastFrameTime = millis();

    currentState = GameState::PLAYING;
}

// -------------------------------------------------------------------------
// Next wave (called when all aliens are cleared; preserves score + lives)
// -------------------------------------------------------------------------

void SpaceInvadersApp::nextWave() {
    level++;
    alienMoveInterval = max(100, 500 - (level - 1) * 50);

    // Reset alien grid
    for (int i = 0; i < NUM_ALIENS; i++) {
        aliens[i].x      = 20 + (i % 5) * 25;
        aliens[i].y      = 25 + (i / 5) * 15;
        aliens[i].active = true;
        aliens[i].type   = i / 5;
    }

    alienDirection = 1;
    animationFrame = 0;
    bullet.active  = false;
    for (auto& b : bombs) b.active = false;
    ufoActive      = false;
    nextUfoTime    = millis() + random(10000, 20000);
    lastAlienMove  = millis();
    lastAlienAnim  = millis();

    stateTimer   = millis();
    currentState = GameState::WAVE_START;
    drawWaveStartScreen();
}

// -------------------------------------------------------------------------
// Persist high-score to NVS if current score beats it
// -------------------------------------------------------------------------

void SpaceInvadersApp::saveHighScore() {
    if (score > highScore) {
        highScore = score;
        Preferences prefs;
        prefs.begin("invaders", false);
        prefs.putInt("hiscore", highScore);
        prefs.end();
    }
}

// -------------------------------------------------------------------------
// Update loop
// -------------------------------------------------------------------------

void SpaceInvadersApp::update() {
    // --- Exit combo (works in ALL states) ---
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);
    bool          comboActive = (durSel > 0 && durL2 > 0 && durR2 > 0);
    unsigned long comboTime   = comboActive ? min(durSel, min(durL2, durR2)) : 0;

    if (comboActive && comboTime >= 2000) {
        display->clearProgressBar();
        appNavigator->switchApp(AppId::Menu);
        return;
    }

    // --- TITLE ---
    if (currentState == GameState::TITLE) {
        updateTitleAnimation();
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- PAUSED: static screen, only combo bar updates ---
    if (currentState == GameState::PAUSED) {
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        } else {
            display->clearProgressBar();
        }
        return;
    }

    // --- GAME OVER: static screen, only combo bar updates ---
    if (currentState == GameState::GAMEOVER) {
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        } else {
            display->clearProgressBar();
        }
        return;
    }

    // --- DYING: overlay shown for 1.5 s, then respawn or game over ---
    if (currentState == GameState::DYING) {
        if (millis() - stateTimer >= 1500) {
            if (lives > 0) {
                // Respawn: reset player & clear projectiles
                player.x      = GAME_W / 2 - 8;
                player.y      = GAME_H - 15;
                bullet.active = false;
                for (auto& b : bombs) b.active = false;
                lastFrameTime = millis();
                currentState  = GameState::PLAYING;
            } else {
                saveHighScore();
                currentState = GameState::GAMEOVER;
                drawGameOverScreen();
            }
        }
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- WAVE_START: "WAVE X" screen shown for 2 s ---
    if (currentState == GameState::WAVE_START) {
        if (millis() - stateTimer >= 2000) {
            lastFrameTime = millis();
            currentState  = GameState::PLAYING;
        }
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- PLAYING: 30 ms frame cap ---
    if (millis() - lastFrameTime < 30) return;
    lastFrameTime = millis();

    // -- Alien animation (250 ms cadence) --
    if (millis() - lastAlienAnim > 250) {
        lastAlienAnim  = millis();
        animationFrame = !animationFrame;
    }

    // -- Alien movement: classic speed curve --
    //    effectiveInterval scales with remaining aliens so the last few
    //    survivors move noticeably faster (as in the original cabinet).
    int remaining = 0;
    for (auto& a : aliens) if (a.active) remaining++;
    int effectiveInterval = max(50, (alienMoveInterval * remaining) / NUM_ALIENS);

    if (millis() - lastAlienMove > effectiveInterval) {
        lastAlienMove = millis();

        bool hitEdge = false;
        for (auto& a : aliens) {
            if (!a.active) continue;
            a.x += alienDirection * 2;
            if (a.x <= 5 || a.x >= GAME_W - 20) hitEdge = true;
        }
        if (hitEdge) {
            alienDirection *= -1;
            for (auto& a : aliens) if (a.active) a.y += 8;
        }

        // Random bomb drop
        if (random(10) > 7) {
            int shooter = random(NUM_ALIENS);
            if (aliens[shooter].active) {
                for (auto& b : bombs) {
                    if (!b.active) {
                        b.fire(aliens[shooter].x + 7, aliens[shooter].y + 8, 3);
                        break;
                    }
                }
            }
        }
    }

    // -- UFO --
    if (!ufoActive && millis() >= nextUfoTime) {
        ufoActive = true;
        ufoX      = -16;
    }
    if (ufoActive) {
        ufoX += 2;
        if (ufoX > GAME_W) {
            ufoActive   = false;
            nextUfoTime = millis() + random(10000, 20000);
        }
    }

    // -- Update projectiles --
    bullet.update();
    for (auto& b : bombs) b.update();

    // -- Bullet vs aliens --
    if (bullet.active) {
        for (auto& a : aliens) {
            if (a.checkHit(bullet.x, bullet.y)) {
                bullet.active = false;
                score++;
                sound->play(SoundEffect::EXPLOSION);
                // Check wave cleared (recount — one alien just became inactive)
                bool allDead = true;
                for (auto& check : aliens) if (check.active) { allDead = false; break; }
                if (allDead) { nextWave(); return; }
                break;
            }
        }
    }

    // -- Bullet vs UFO --
    if (bullet.active && ufoActive &&
        bullet.x >= ufoX  && bullet.x <= ufoX + 16 &&
        bullet.y >= UFO_Y && bullet.y <= UFO_Y + 8) {
        bullet.active = false;
        ufoActive     = false;
        score        += random(1, 7) * 50;   // 50 / 100 / 150 / 200 / 250 / 300
        sound->play(SoundEffect::EXPLOSION);
        nextUfoTime = millis() + random(10000, 20000);
    }

    // -- Bombs vs player --
    for (auto& b : bombs) {
        if (b.active &&
            b.x >= player.x && b.x <= player.x + 16 &&
            b.y >= player.y && b.y <= player.y + 8) {
            sound->play(SoundEffect::EXPLOSION);
            b.active = false;
            lives--;
            stateTimer   = millis();
            currentState = GameState::DYING;
            drawDyingScreen();
            return;
        }
    }

    // -- Player movement --
    if (moveLeft)  player.move(-3, GAME_W);
    if (moveRight) player.move( 3, GAME_W);

    // -----------------------------------------------------------------------
    // DRAW FRAME  —  clear -> HUD -> entities -> optional combo bar -> flush
    // -----------------------------------------------------------------------
    display->clear();
    drawHud();
    drawAliens();
    player.draw(display, Colors::GREEN);
    bullet.draw(display, Colors::WHITE);
    for (auto& b : bombs) b.draw(display, Colors::ORANGE);
    if (ufoActive) drawUfo(Colors::RED);

    if (comboActive) display->drawProgressBar(comboTime, 2000, Colors::RED);

    display->flush();
}

// -------------------------------------------------------------------------
// Input handler
// -------------------------------------------------------------------------

void SpaceInvadersApp::onInput(ControlEvent ev, EventType type) {
    bool pressed = (type == EventType::PRESSED);

    // Continuous movement — track both edges
    if (ev == ControlEvent::JOY_LEFT)  moveLeft  = pressed;
    if (ev == ControlEvent::JOY_RIGHT) moveRight = pressed;

    if (!pressed) return;

    switch (currentState) {
        case GameState::TITLE:
            if (ev == ControlEvent::BTN_START) initGame();
            break;

        case GameState::PLAYING:
            if (ev == ControlEvent::BTN_START) {
                currentState = GameState::PAUSED;
                drawPauseScreen();
            } else if (ev == ControlEvent::BTN_A && !bullet.active) {
                bullet.fire(player.x + 7, player.y, -5);
                sound->play(SoundEffect::LASER);
            }
            break;

        case GameState::PAUSED:
            if (ev == ControlEvent::BTN_START)
                currentState = GameState::PLAYING;
            break;

        case GameState::GAMEOVER:
            if (ev == ControlEvent::BTN_START) {
                currentState = GameState::TITLE;
                drawTitleScreen();
            }
            break;

        default:
            break;   // DYING / WAVE_START: ignore input
    }
}

// -------------------------------------------------------------------------
// Drawing helpers
// -------------------------------------------------------------------------

void SpaceInvadersApp::drawHud() {
    // Top row: score (left) | hi-score (centre) | level (right)
    display->drawText(2,   2, "SC:" + String(score),     Colors::WHITE);
    display->drawText(58,  2, "HI:" + String(highScore), Colors::YELLOW);
    display->drawText(128, 2, "L:"  + String(level),     Colors::WHITE);

    // Life pips: small green bars just below the player (y=122)
    for (int i = 0; i < min(lives, 5); i++) {
        display->fillRect(2 + i * 8, 122, 6, 4, Colors::GREEN);
    }
}

void SpaceInvadersApp::drawTitleScreen() {
    // Reset animation state so the next updateTitleAnimation() starts fresh
    titleX = 20; titleDir = 1; titleFrame = false; lastTitleAnim = 0;

    display->clear();
    display->drawText(50,  10, "SPACE",               Colors::GREEN, 2);
    display->drawText(32,  30, "INVADERS",            Colors::GREEN, 2);
    display->drawText(38, 115, "PRESS START", Colors::WHITE);
    display->flush();
}

void SpaceInvadersApp::updateTitleAnimation() {
    if (millis() - lastTitleAnim < 100) return;
    lastTitleAnim = millis();

    // Only clear the alien marquee row — title text stays intact in the buffer
    display->fillRect(0, 60, GAME_W, 30, Colors::BLACK);

    titleX += titleDir * 2;
    if (titleX <= 5 || titleX >= GAME_W - 110) titleDir *= -1;
    titleFrame = !titleFrame;

    for (int i = 0; i < 3; i++) {
        int xPos = titleX + i * 40;
        const unsigned char* sprite;
        if      (i == 0) sprite = titleFrame ? alien_squid_1   : alien_squid_2;
        else if (i == 1) sprite = titleFrame ? alien_crab_1    : alien_crab_2;
        else             sprite = titleFrame ? alien_octopus_1 : alien_octopus_2;

        // Manual 2x scaling: read PROGMEM row, map each set bit to a 2x2 block
        for (int y = 0; y < 8; y++) {
            uint16_t row = (pgm_read_byte(&sprite[y * 2]) << 8)
                         |  pgm_read_byte(&sprite[y * 2 + 1]);
            for (int x = 0; x < 16; x++) {
                if (row & (1 << (15 - x))) {
                    display->fillRect(xPos + x * 2, 62 + y * 2, 2, 2, Colors::RED);
                }
            }
        }
    }

    display->flush();
}

void SpaceInvadersApp::drawPauseScreen() {
    // Overlay box on top of the frozen game frame
    display->fillRect(30, 40, 100, 40, Colors::BLACK);
    display->drawRect(30, 40, 100, 40, Colors::WHITE);
    display->drawText(44, 52, "PAUSED", Colors::YELLOW, 2);
    display->flush();
}

void SpaceInvadersApp::drawGameOverScreen() {
    display->clear();
    display->drawText(26,  20, "GAME OVER",                    Colors::RED,    2);
    display->drawText(30,  55, "Score: " + String(score),      Colors::WHITE);
    display->drawText(30,  70, "Best:  " + String(highScore),  Colors::YELLOW);
    display->drawText(45, 100, "PRESS START",                  Colors::WHITE);
    display->flush();
}

void SpaceInvadersApp::drawDyingScreen() {
    // Overlay on the frozen game frame (buffer still holds last rendered frame)
    display->fillRect(20, 44, 120, 40, Colors::BLACK);
    display->drawRect(20, 44, 120, 40, Colors::RED);
    display->drawText(28, 52, "SHIP LOST!", Colors::RED,   2);
    display->drawText(48, 72, "LIVES: " + String(lives),  Colors::WHITE);
    display->flush();
}

void SpaceInvadersApp::drawWaveStartScreen() {
    display->clear();
    display->drawText(28, 35, "WAVE " + String(level), Colors::GREEN,  2);
    display->drawText(25, 65, "SCORE: " + String(score),               Colors::WHITE);
    display->drawText(28, 80, "HI:    " + String(highScore),           Colors::YELLOW);
    display->flush();
}

void SpaceInvadersApp::drawAliens() {
    for (auto& a : aliens) a.draw(display, animationFrame);
}

void SpaceInvadersApp::drawUfo(uint16_t color) {
    display->drawBitmap(ufoX, UFO_Y, ufo_bmp, 16, 8, color);
}
