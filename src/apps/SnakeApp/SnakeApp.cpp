/**
 * Project: Arcade Controller V1.3
 * File: apps/SnakeApp/SnakeApp.cpp
 * Description: Snake game logic and rendering.
 *
 *              Rendering: clear() -> draw HUD + grid + food + snake -> flush()
 *              every PLAYING frame, identical to SpaceInvadersApp.
 *
 *              Snake storage: flat int arrays, head at index 0.
 *              On every move the array is shifted right by one; on food pickup
 *              the array is extended by one before shifting so the tail stays.
 *
 *              Speed formula:  moveInterval = max(60, max(80, 200-snakeLen*2) - (level-1)*8)
 *                Level 1, len  3 -> 194 ms  (~5 steps/s)
 *                Level 3, len 20 -> 144 ms  (~7 steps/s)
 *                Level 5, len 50 ->   68 ms (near floor, ~15 steps/s)
 *
 *              Level mechanic:
 *                Eat PILLS_PER_LEVEL pills -> exit spawns (YELLOW ring)
 *                Reach exit -> LEVEL_COMPLETE overlay (1.5 s) -> next level
 *                Each level adds wall obstacles (4 distinct layouts, cycling).
 *
 *              Title animation:
 *                10-segment demo snake slithers in a serpentine boustrophedon
 *                pattern between DEMO_TOP and DEMO_BOT, 100 ms per step.
 *                Only the animation band is cleared each frame so "SNAKE" and
 *                "PRESS START" (drawn once) stay intact in the sprite buffer.
 */

#include "SnakeApp.h"
#include "../../config/Colors.h"
#include <Preferences.h>

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------

SnakeApp::SnakeApp(IDisplay*      display,
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

void SnakeApp::start() {
    Serial.println("[APP] SnakeApp starting...");

    Preferences prefs;
    prefs.begin("snake", true);
    highScore = prefs.getInt("hiscore", 0);
    prefs.end();

    currentState = SnakeState::TITLE;
    drawTitleScreen();
}

void SnakeApp::stop() {
    display->clearProgressBar();
}

// -------------------------------------------------------------------------
// Game initialisation
// -------------------------------------------------------------------------

void SnakeApp::initGame() {
    score          = 0;
    level          = 1;
    initLevel();
}

void SnakeApp::initLevel() {
    // Clear walls then load layout for current level
    memset(walls, 0, sizeof(walls));   // zeros all bits in the uint32_t array
    loadWalls();

    // Snake: 3 segments at col 9/8/7, row 7, heading right
    snakeLen    = 3;
    snakeX[0]   = 9;  snakeY[0] = 7;
    snakeX[1]   = 8;  snakeY[1] = 7;
    snakeX[2]   = 7;  snakeY[2] = 7;

    dir         = SnakeDir::RIGHT;
    dirQHead    = 0;   // flush input queue
    dirQTail    = 0;

    pillsThisLevel = 0;
    exitActive     = false;

    placeFood();

    lastMove      = millis();
    lastFrameTime = millis();
    currentState  = SnakeState::PLAYING;
}

// -------------------------------------------------------------------------
// Wall layouts (4 distinct, cycling with ((level-1) % 4) + 1 )
// -------------------------------------------------------------------------

void SnakeApp::loadWalls() {
    int layout = ((level - 1) % 4) + 1;   // 1..4

    switch (layout) {
        case 1:
            // Level 1: no walls
            break;

        case 2: {
            // Level 2: two horizontal bars (rows 4 & 9, cols 3..16)
            for (int c = 3; c <= 16; c++) {
                setWall(4, c);
                setWall(9, c);
            }
            break;
        }

        case 3: {
            // Level 3: four L-shaped corner obstacles
            for (int c = 2; c <= 6;  c++) setWall(2, c);   // top-left H
            for (int r = 2; r <= 5;  r++) setWall(r, 2);   // top-left V
            for (int c = 13; c <= 17; c++) setWall(2, c);  // top-right H
            for (int r = 2; r <= 5;  r++) setWall(r, 17);  // top-right V
            for (int c = 2; c <= 6;  c++) setWall(11, c);  // bot-left H
            for (int r = 8; r <= 11; r++) setWall(r, 2);   // bot-left V
            for (int c = 13; c <= 17; c++) setWall(11, c); // bot-right H
            for (int r = 8; r <= 11; r++) setWall(r, 17);  // bot-right V
            break;
        }

        case 4: {
            // Level 4: zigzag corridor.
            // Spawn is at cols 7-9, row 7 heading RIGHT — rows 5-9 stay clear.
            // Top bar (row 3): cols 1-14, gap on the right (cols 15-18)
            for (int c = 1; c <= 14; c++) setWall(3, c);
            // Bottom bar (row 11): cols 5-18, gap on the left (cols 0-4)
            for (int c = 5; c <= 18; c++) setWall(11, c);
            break;
        }
    }
}

// -------------------------------------------------------------------------
// Food placement — random free cell (not wall, not snake)
// -------------------------------------------------------------------------

void SnakeApp::placeFood() {
    for (int attempt = 0; attempt < 300; attempt++) {
        int fx = random(COLS);
        int fy = random(ROWS);
        if (isWall(fy, fx)) continue;
        bool occupied = false;
        for (int i = 0; i < snakeLen; i++) {
            if (snakeX[i] == fx && snakeY[i] == fy) { occupied = true; break; }
        }
        if (!occupied) { foodX = fx; foodY = fy; return; }
    }
    // Fallback: linear scan
    for (int fy = 0; fy < ROWS; fy++) {
        for (int fx = 0; fx < COLS; fx++) {
            if (isWall(fy, fx)) continue;
            bool occupied = false;
            for (int i = 0; i < snakeLen; i++) {
                if (snakeX[i] == fx && snakeY[i] == fy) { occupied = true; break; }
            }
            if (!occupied) { foodX = fx; foodY = fy; return; }
        }
    }
}

// -------------------------------------------------------------------------
// Exit spawn — random free cell not on wall, not on snake, not on food
// -------------------------------------------------------------------------

void SnakeApp::spawnExit() {
    for (int attempt = 0; attempt < 300; attempt++) {
        int ex = random(COLS);
        int ey = random(ROWS);
        if (isWall(ey, ex)) continue;
        if (ex == foodX && ey == foodY) continue;
        bool onSnake = false;
        for (int i = 0; i < snakeLen; i++) {
            if (snakeX[i] == ex && snakeY[i] == ey) { onSnake = true; break; }
        }
        if (!onSnake) {
            exitX       = ex;
            exitY       = ey;
            exitActive  = true;
            return;
        }
    }
    // Fallback
    for (int ey = 0; ey < ROWS; ey++) {
        for (int ex = 0; ex < COLS; ex++) {
            if (isWall(ey, ex)) continue;
            if (ex == foodX && ey == foodY) continue;
            bool onSnake = false;
            for (int i = 0; i < snakeLen; i++) {
                if (snakeX[i] == ex && snakeY[i] == ey) { onSnake = true; break; }
            }
            if (!onSnake) { exitX = ex; exitY = ey; exitActive = true; return; }
        }
    }
}

// -------------------------------------------------------------------------
// NVS high-score
// -------------------------------------------------------------------------

void SnakeApp::saveHighScore() {
    if (score > highScore) {
        highScore = score;
        Preferences prefs;
        prefs.begin("snake", false);
        prefs.putInt("hiscore", highScore);
        prefs.end();
    }
}

// -------------------------------------------------------------------------
// Speed scaling
// -------------------------------------------------------------------------

int SnakeApp::moveInterval() const {
    int base = max(80, 200 - snakeLen * 2);
    return max(60, base - (level - 1) * 8);
}

// -------------------------------------------------------------------------
// Title-screen: initialise demo snake at centre, draw static text
// -------------------------------------------------------------------------

void SnakeApp::drawTitleScreen() {
    // Init demo snake — 10 segments in a row, heading right, at vertical centre
    int startRow = (DEMO_TOP + DEMO_BOT) / 2;   // pixel y ~66 -> row in screen coords
    demoDirX = 1;
    for (int i = 0; i < DEMO_LEN; i++) {
        demoX[i] = 70 - i * CELL;   // head at x=70, tail trails left
        demoY[i] = startRow;
    }
    lastTitleAnim = 0;

    display->clear();
    // "SNAKE" top-centre (size 2 → 12 px high, x centred at 160/2-30=50)
    display->drawText(50, 10, "SNAKE", Colors::GREEN, 2);
    // "PRESS START" bottom-centre (size 1, small white)
    display->drawText(38, 115, "PRESS START", Colors::WHITE);
    display->flush();
}

// -------------------------------------------------------------------------
// Title animation: step demo snake every 100 ms, clear only the band
// -------------------------------------------------------------------------

void SnakeApp::updateTitleAnimation() {
    if (millis() - lastTitleAnim < 100) return;
    lastTitleAnim = millis();

    // Clear just the animation band — text outside the band stays intact
    const int BAND_Y = DEMO_TOP - 2;
    const int BAND_H = DEMO_BOT - DEMO_TOP + CELL + 2;
    display->fillRect(0, BAND_Y, 160, BAND_H, Colors::BLACK);

    // --- Step: shift body segments toward the tail, new head = one step forward ---
    for (int i = DEMO_LEN - 1; i > 0; i--) {
        demoX[i] = demoX[i - 1];
        demoY[i] = demoY[i - 1];
    }

    // Compute new head position
    int nx = demoX[1] + demoDirX * CELL;   // one step in X
    int ny = demoY[1];                      // same row

    // Hit right wall -> move down one row, reverse
    if (demoDirX > 0 && nx > 160 - CELL) {
        nx        = demoX[1];   // stay in column
        ny        = demoY[1] + CELL;
        demoDirX  = -1;
    }
    // Hit left wall -> move down one row, reverse
    else if (demoDirX < 0 && nx < 0) {
        nx        = demoX[1];
        ny        = demoY[1] + CELL;
        demoDirX  = 1;
    }

    // Wrap Y back to DEMO_TOP when past DEMO_BOT
    if (ny > DEMO_BOT) ny = DEMO_TOP;

    demoX[0] = nx;
    demoY[0] = ny;

    // --- Draw segments (tail to head so head is on top) ---
    for (int i = DEMO_LEN - 1; i >= 0; i--) {
        int px = demoX[i];
        int py = demoY[i];
        // Clip to band
        if (py < BAND_Y || py > BAND_Y + BAND_H) continue;
        if (i == 0) {
            display->fillRect(px,     py,     CELL,     CELL,     Colors::GREEN);
        } else {
            display->fillRect(px + 1, py + 1, CELL - 2, CELL - 2, Colors::GREEN);
        }
    }

    display->flush();
}

// -------------------------------------------------------------------------
// Update loop
// -------------------------------------------------------------------------

void SnakeApp::update() {
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

    // --- TITLE: animate demo snake ---
    if (currentState == SnakeState::TITLE) {
        updateTitleAnimation();
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- GAMEOVER: static screen ---
    if (currentState == SnakeState::GAMEOVER) {
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- LEVEL_COMPLETE: wait 1.5 s then advance ---
    if (currentState == SnakeState::LEVEL_COMPLETE) {
        if (millis() - stateTimer >= 1500) {
            level++;
            initLevel();
        }
        if (comboActive) {
            display->drawProgressBar(comboTime, 2000, Colors::RED);
            display->flush();
        }
        return;
    }

    // --- PLAYING: 30 ms render cap ---
    if (millis() - lastFrameTime < 30) return;
    lastFrameTime = millis();

    // --- Move snake at scaled interval ---
    if (millis() - lastMove >= (unsigned long)moveInterval()) {
        lastMove = millis();

        // Consume one direction from the queue — block 180° reversals.
        // 180° entries are discarded and the next queued entry is tried
        // so a bad press never stalls the queue.
        while (dirQHead != dirQTail) {
            SnakeDir candidate = dirQueue[dirQHead];
            dirQHead = (dirQHead + 1) % DIR_QUEUE_SIZE;
            bool reverse =
                (candidate == SnakeDir::UP    && dir == SnakeDir::DOWN)  ||
                (candidate == SnakeDir::DOWN  && dir == SnakeDir::UP)    ||
                (candidate == SnakeDir::LEFT  && dir == SnakeDir::RIGHT) ||
                (candidate == SnakeDir::RIGHT && dir == SnakeDir::LEFT);
            if (!reverse) { dir = candidate; break; }
        }

        // Compute new head cell
        int nx = snakeX[0], ny = snakeY[0];
        if      (dir == SnakeDir::UP)    ny--;
        else if (dir == SnakeDir::DOWN)  ny++;
        else if (dir == SnakeDir::LEFT)  nx--;
        else                             nx++;

        // Border collision -> game over
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) {
            sound->play(SoundEffect::EXPLOSION);
            saveHighScore();
            currentState = SnakeState::GAMEOVER;
            drawGameOverScreen();
            return;
        }

        // Wall collision -> game over
        if (isWall(ny, nx)) {
            sound->play(SoundEffect::EXPLOSION);
            saveHighScore();
            currentState = SnakeState::GAMEOVER;
            drawGameOverScreen();
            return;
        }

        // Self collision (skip last segment — it vacates this step)
        for (int i = 0; i < snakeLen - 1; i++) {
            if (snakeX[i] == nx && snakeY[i] == ny) {
                sound->play(SoundEffect::EXPLOSION);
                saveHighScore();
                currentState = SnakeState::GAMEOVER;
                drawGameOverScreen();
                return;
            }
        }

        // Check exit BEFORE food
        if (exitActive && nx == exitX && ny == exitY) {
            // Level complete — shift body normally (no grow)
            for (int i = snakeLen - 1; i > 0; i--) {
                snakeX[i] = snakeX[i - 1];
                snakeY[i] = snakeY[i - 1];
            }
            snakeX[0] = nx;
            snakeY[0] = ny;

            saveHighScore();
            stateTimer   = millis();
            currentState = SnakeState::LEVEL_COMPLETE;
            drawLevelCompleteScreen();
            return;
        }

        // Check food
        bool ate = (nx == foodX && ny == foodY);

        if (ate && snakeLen < MAX_LEN) {
            // Grow: extend array right by one, then shift
            for (int i = snakeLen; i > 0; i--) {
                snakeX[i] = snakeX[i - 1];
                snakeY[i] = snakeY[i - 1];
            }
            snakeLen++;
            score++;
            pillsThisLevel++;
            sound->play(SoundEffect::LASER);
            placeFood();

            // Enough pills eaten? Spawn exit portal
            if (!exitActive && pillsThisLevel >= PILLS_PER_LEVEL) {
                spawnExit();
            }
        } else {
            // Normal move: tail falls off
            for (int i = snakeLen - 1; i > 0; i--) {
                snakeX[i] = snakeX[i - 1];
                snakeY[i] = snakeY[i - 1];
            }
        }
        snakeX[0] = nx;
        snakeY[0] = ny;
    }

    // -----------------------------------------------------------------------
    // DRAW FRAME
    // -----------------------------------------------------------------------
    drawGame();
    if (comboActive) display->drawProgressBar(comboTime, 2000, Colors::RED);
    display->flush();
}

// -------------------------------------------------------------------------
// Input handler
// -------------------------------------------------------------------------

void SnakeApp::onInput(ControlEvent ev, EventType type) {
    if (type != EventType::PRESSED) return;

    switch (currentState) {
        case SnakeState::TITLE:
            if (ev == ControlEvent::BTN_START || ev == ControlEvent::BTN_A)
                initGame();
            break;

        case SnakeState::PLAYING: {
            SnakeDir newDir = dir;   // default: no change
            if      (ev == ControlEvent::JOY_UP)    newDir = SnakeDir::UP;
            else if (ev == ControlEvent::JOY_DOWN)  newDir = SnakeDir::DOWN;
            else if (ev == ControlEvent::JOY_LEFT)  newDir = SnakeDir::LEFT;
            else if (ev == ControlEvent::JOY_RIGHT) newDir = SnakeDir::RIGHT;
            else break;   // not a directional event

            // Push into queue if not full
            int nextTail = (dirQTail + 1) % DIR_QUEUE_SIZE;
            if (nextTail != dirQHead) {
                dirQueue[dirQTail] = newDir;
                dirQTail = nextTail;
            }
            break;
        }

        case SnakeState::GAMEOVER:
            if (ev == ControlEvent::BTN_START || ev == ControlEvent::BTN_A) {
                currentState = SnakeState::TITLE;
                drawTitleScreen();
            }
            break;

        case SnakeState::LEVEL_COMPLETE:
            // Input is ignored during the level-complete overlay
            break;
    }
}

// -------------------------------------------------------------------------
// drawGame — full frame: progress bar, HUD, walls, exit, food, snake
// -------------------------------------------------------------------------

void SnakeApp::drawGame() {
    display->clear();

    // --- 2 px pill progress bar at y = 0 ---
    // Full width = 160 px. Yellow when exit active, green otherwise.
    int   barMax   = PILLS_PER_LEVEL;
    int   barVal   = exitActive ? PILLS_PER_LEVEL : pillsThisLevel;
    int   barColor = exitActive ? Colors::YELLOW  : Colors::GREEN;
    int   barW     = (barVal * 160) / barMax;
    if (barW > 0) display->fillRect(0, 0, barW, 2, barColor);

    // --- HUD at y=2 (12 px band) ---
    display->drawText(2,   2, "SC:" + String(score),     Colors::WHITE);
    display->drawText(60,  2, "HI:" + String(highScore), Colors::YELLOW);
    display->drawText(128, 2, "L:"  + String(level),     Colors::WHITE);

    // --- Walls (grey filled cells) ---
    for (int r = 0; r < ROWS; r++) {
        if (!walls[r]) continue;   // fast skip: no walls in this row
        for (int c = 0; c < COLS; c++) {
            if (isWall(r, c)) {
                display->fillRect(
                    GRID_X + c * CELL,
                    GRID_Y + r * CELL,
                    CELL, CELL,
                    Colors::GREY);
            }
        }
    }

    // --- Exit portal: YELLOW ring (outer cell filled, inner area BLACK) ---
    if (exitActive) {
        int px = GRID_X + exitX * CELL;
        int py = GRID_Y + exitY * CELL;
        display->fillRect(px,     py,     CELL,     CELL,     Colors::YELLOW);
        display->fillRect(px + 2, py + 2, CELL - 4, CELL - 4, Colors::BLACK);
    }

    // --- Food: 4×4 red square centred in cell ---
    display->fillRect(
        GRID_X + foodX * CELL + 2,
        GRID_Y + foodY * CELL + 2,
        4, 4, Colors::RED);

    // --- Snake (tail → head so head pixel is always on top) ---
    for (int i = snakeLen - 1; i >= 0; i--) {
        int px = GRID_X + snakeX[i] * CELL;
        int py = GRID_Y + snakeY[i] * CELL;
        if (i == 0) {
            display->fillRect(px,     py,     CELL,     CELL,     Colors::GREEN);
        } else {
            display->fillRect(px + 1, py + 1, CELL - 2, CELL - 2, Colors::GREEN);
        }
    }
}

// -------------------------------------------------------------------------
// drawTitleScreen — called once; update loop drives the animation from here
// -------------------------------------------------------------------------
// (implemented above, before update())

// -------------------------------------------------------------------------
// drawLevelCompleteScreen — overlay box
// -------------------------------------------------------------------------

void SnakeApp::drawLevelCompleteScreen() {
    display->clear();
    // Background box
    display->fillRect(20, 40, 120, 50, Colors::BLACK);
    display->drawRect(20, 40, 120, 50, Colors::YELLOW);
    display->drawText(40, 48, "LEVEL " + String(level), Colors::GREEN,  2);
    display->drawText(52, 72, "CLEAR!",                 Colors::YELLOW, 2);
    display->flush();
}

// -------------------------------------------------------------------------
// drawGameOverScreen
// -------------------------------------------------------------------------

void SnakeApp::drawGameOverScreen() {
    display->clear();
    display->drawText(20,  20, "GAME OVER",                   Colors::RED,    2);
    display->drawText(30,  55, "Score: " + String(score),     Colors::WHITE);
    display->drawText(30,  70, "Best:  " + String(highScore), Colors::YELLOW);
    display->drawText(25, 100, "PRESS START",                 Colors::WHITE);
    display->flush();
}
