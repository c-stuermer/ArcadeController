/**
 * Project: Arcade Controller V1.3
 * File: apps/SpaceInvadersApp/SpaceInvadersApp.h
 * Description: Retro Space Invaders mini-game.
 *
 *              State machine:
 *                TITLE -> PLAYING <-> PAUSED
 *                PLAYING -> DYING      (bomb hit, lives--)
 *                  DYING -> PLAYING    (lives > 0, respawn after 1.5 s)
 *                  DYING -> GAMEOVER   (lives == 0)
 *                PLAYING -> WAVE_START (all aliens cleared)
 *                  WAVE_START -> PLAYING (after 2 s)
 *                GAMEOVER -> TITLE
 *
 *              SELECT + L2 + R2 (2 s hold) returns to the main menu from
 *              any state.
 *
 *              V1.3+ additions:
 *                - 3 lives (DYING state + respawn)
 *                - Level progression: alienMoveInterval -50 ms/wave (min 100)
 *                - Classic speed-up: interval scales with remaining/total aliens
 *                - NVS high-score via Preferences ("invaders" / "hiscore")
 *                - UFO awards random 50-300 pts instead of fixed 5
 *                - HUD: score | hi-score | level  +  life pips at bottom
 */

#pragma once
#include "../App.h"
#include "../Interfaces/IAppNavigator.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/ISound.h"
#include "../../hal/interfaces/IInputH.h"
#include <Arduino.h>
#include "SpaceInvadersEntities.h"

enum class GameState { TITLE, PLAYING, PAUSED, GAMEOVER, DYING, WAVE_START };

class SpaceInvadersApp : public App {
public:
    SpaceInvadersApp(IDisplay*      display,
                     ISound*        sound,
                     IInputH*       input,
                     IAppNavigator* appNavigator);

    void start()   override;
    void update()  override;
    void stop()    override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    IDisplay*      display;
    ISound*        sound;
    IInputH*       input;
    IAppNavigator* appNavigator;

    GameState currentState = GameState::TITLE;

    // Entities
    PlayerEntity player;
    static const int NUM_ALIENS = 15;
    AlienEntity  aliens[NUM_ALIENS];
    Projectile   bullet;
    Projectile   bombs[3];

    // Game-logic state
    bool moveLeft          = false;
    bool moveRight         = false;
    int  score             = 0;
    int  highScore         = 0;
    int  lives             = 3;
    int  level             = 1;
    int  alienDirection    = 1;
    int  animationFrame    = 0;
    int  alienMoveInterval = 500;   // ms; decreases 50 ms per wave, floor 100

    unsigned long lastAlienMove = 0;
    unsigned long lastAlienAnim = 0;   // decoupled from movement: 250 ms cadence
    unsigned long lastFrameTime = 0;
    unsigned long stateTimer    = 0;   // DYING / WAVE_START duration tracking

    // UFO (mothership)
    int           ufoX       = -16;
    bool          ufoActive  = false;
    unsigned long nextUfoTime = 0;

    // Title animation
    unsigned long lastTitleAnim = 0;
    int           titleX        = 20;
    int           titleDir      = 1;
    bool          titleFrame    = false;

    // Internal helpers
    void initGame();
    void nextWave();
    void saveHighScore();

    void drawHud();
    void drawAliens();
    void drawUfo(uint16_t color);
    void drawTitleScreen();
    void updateTitleAnimation();
    void drawPauseScreen();
    void drawGameOverScreen();
    void drawDyingScreen();
    void drawWaveStartScreen();
};
