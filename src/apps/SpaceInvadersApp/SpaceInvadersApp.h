/**
 * Project: Arcade Controller V1.0
 * File: apps/SpaceInvadersApp/SpaceInvadersApp.h
 * Description: Retro mini-game (Space Invaders clone). State machine:
 *              TITLE -> PLAYING -> PAUSED / GAMEOVER. SELECT+L2+R2 (2s)
 *              returns to the main menu from any state.
 */

#pragma once
#include "../App.h"
#include <Arduino.h>
#include "SpaceInvadersEntities.h"

// GameState must be defined before the class so members can use it.
enum class GameState { TITLE, PLAYING, PAUSED, GAMEOVER };

class SpaceInvadersApp : public App {
private:
    GameState currentState;

    // Entities
    PlayerEntity player;
    static const int NUM_ALIENS = 15;
    AlienEntity aliens[NUM_ALIENS];
    Projectile bullet;
    Projectile bombs[3];

    // Game-logic state
    bool moveLeft = false;
    bool moveRight = false;
    int score = 0;
    int alienDirection = 1;
    int animationFrame = 0;

    unsigned long lastAlienMove = 0;
    unsigned long lastBombDrop = 0;
    unsigned long lastFrameTime = 0;

    // UFO logic
    int ufoX = -16;
    bool ufoActive = false;
    unsigned long nextUfoTime = 0;

    // Internal helpers
    void initGame();
    void drawAliens();
    void drawUfo(uint16_t color);
    void updateTitleAnimation();

    void drawTitleScreen();
    void drawPauseScreen();
    void drawGameOverScreen();

public:
    using App::App;
    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;
};
