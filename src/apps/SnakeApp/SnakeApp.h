/**
 * Project: Arcade Controller V1.3
 * File: apps/SnakeApp/SnakeApp.h
 * Description: Classic Snake mini-game with level progression.
 *
 *              Grid: 20 x 14 cells at 8 px each (160 x 112 px).
 *              12 px HUD above the grid (score / hi-score / level).
 *              2 px pill-progress bar at the very top (y=0).
 *
 *              State machine:
 *                TITLE -> PLAYING -> LEVEL_COMPLETE -> PLAYING (next level)
 *                PLAYING -> GAMEOVER -> TITLE
 *
 *              Level mechanics:
 *                - Eat PILLS_PER_LEVEL (20) pills -> exit spawns (YELLOW ring)
 *                - Reach the exit -> LEVEL_COMPLETE (1.5 s overlay) -> next level
 *                - Each level adds wall obstacles (4 distinct layouts, cycling)
 *                - Speed increases 8 ms per level on top of length-based scaling
 *
 *              Title screen:
 *                - "SNAKE" top-centre in green
 *                - Animated demo-snake (10 segments) slithering in a serpentine
 *                  pattern across the middle of the screen
 *                - "PRESS START" bottom-centre in small white text
 *
 *              Controls:
 *                Joystick     — change direction (180 deg reversal blocked)
 *                START / A    — start / restart
 *                SELECT+L2+R2 — return to main menu (2 s hold, any state)
 *
 *              NVS high-score: Preferences namespace "snake", key "hiscore".
 */

#pragma once
#include "../App.h"
#include "../Interfaces/IAppNavigator.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/ISound.h"
#include "../../hal/interfaces/IInputH.h"
#include <Arduino.h>

enum class SnakeState { TITLE, PLAYING, LEVEL_COMPLETE, GAMEOVER };
enum class SnakeDir   { UP, DOWN, LEFT, RIGHT };

class SnakeApp : public App {
public:
    SnakeApp(IDisplay*      display,
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

    // -----------------------------------------------------------------------
    // Grid geometry
    // -----------------------------------------------------------------------
    static constexpr int CELL    = 8;
    static constexpr int COLS    = 20;
    static constexpr int ROWS    = 14;
    static constexpr int GRID_X  = 0;
    static constexpr int GRID_Y  = 12;   // below 12 px HUD
    static constexpr int MAX_LEN = COLS * ROWS;

    // -----------------------------------------------------------------------
    // Game state
    // -----------------------------------------------------------------------
    SnakeState currentState = SnakeState::TITLE;
    SnakeDir   dir          = SnakeDir::RIGHT;

    // Direction input queue — buffers rapid presses so no input is lost
    // between move ticks (e.g. UP then LEFT in quick succession).
    static constexpr int DIR_QUEUE_SIZE = 3;
    SnakeDir dirQueue[DIR_QUEUE_SIZE];
    int      dirQHead = 0;   // next entry to consume
    int      dirQTail = 0;   // next free slot (head == tail -> empty)

    // Cell coordinates fit in uint8_t (x: 0-19, y: 0-13).
    // Using uint8_t halves the array footprint vs int: 560 bytes instead of 2240.
    uint8_t snakeX[COLS * ROWS];
    uint8_t snakeY[COLS * ROWS];
    int     snakeLen = 0;

    uint8_t foodX = 0, foodY = 0;

    int score          = 0;
    int highScore      = 0;
    int level          = 1;
    int pillsThisLevel = 0;

    static constexpr int PILLS_PER_LEVEL = 20;

    bool    exitActive = false;
    uint8_t exitX      = 0, exitY = 0;

    // Bitfield wall grid: bit c of walls[r] = wall present at (col c, row r).
    // 14 × uint32_t = 56 bytes instead of bool[14][20] = 280 bytes.
    uint32_t walls[ROWS];
    bool     isWall (int r, int c) const { return (walls[r] >> c) & 1u; }
    void     setWall(int r, int c)       { walls[r] |= (1u << c); }

    unsigned long lastMove      = 0;
    unsigned long lastFrameTime = 0;
    unsigned long stateTimer    = 0;   // LEVEL_COMPLETE duration

    // -----------------------------------------------------------------------
    // Title-screen demo snake
    // -----------------------------------------------------------------------
    static constexpr int  DEMO_LEN  = 10;
    static constexpr int  DEMO_TOP  = 46;   // pixel y of topmost demo row
    static constexpr int  DEMO_BOT  = 86;   // pixel y of bottommost demo row

    int  demoX[DEMO_LEN], demoY[DEMO_LEN];  // head at index 0
    int  demoDirX       = 1;
    unsigned long lastTitleAnim = 0;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------
    int  moveInterval() const;

    void initGame();
    void initLevel();
    void loadWalls();
    void placeFood();
    void spawnExit();
    void saveHighScore();

    void updateTitleAnimation();

    void drawGame();
    void drawTitleScreen();
    void drawLevelCompleteScreen();
    void drawGameOverScreen();
};
