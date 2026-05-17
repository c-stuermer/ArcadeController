/**
 * Project: Arcade Controller V1.2
 * File: InputMonitorApp.cpp
 * Description: Implementation of the input monitor UI and logic.
 */

#include "InputMonitorApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../../config/Colors.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/InputHandler.h"

// Helper: bit position for a logical input within the debounced-state bitmap.
// Mirrors InputHandler::getDebouncedStates() — see ControlEvent enum.
static constexpr uint16_t EVENT_BIT(ControlEvent e) {
    return 1u << static_cast<int>(e);
}

static constexpr uint16_t JOY_MASK =
      EVENT_BIT(ControlEvent::JOY_UP)   | EVENT_BIT(ControlEvent::JOY_DOWN)
    | EVENT_BIT(ControlEvent::JOY_LEFT) | EVENT_BIT(ControlEvent::JOY_RIGHT);

void InputMonitorApp::start() {
    Serial.println("[APP] InputMonitorApp starting...");
    auto gfx = system->getDisplay()->getGfx();
    gfx->fillScreen(Colors::BLACK);
    
    // --- Static text for exit combo (like in the Bluetooth App) ---
    gfx->setTextSize(1);
    gfx->setTextColor(Colors::RED); // Red
    gfx->drawString("[SELECT] + [L2] + [R2]", 10, 112); 

    lastState = ALL_INPUTS_DIRTY; // Forces a full redraw on first update
}

void InputMonitorApp::stop() {
    // Ensure the progress bar is cleared when exiting the app
    system->getDisplay()->clearProgressBar();
}

void InputMonitorApp::onInput(ControlEvent ev, EventType type) {
    // Inputs are handled directly via polling in update()
}

void InputMonitorApp::update() {
    system->getDisplay()->drawHeader("INPUT MONITOR");

    auto gfx = system->getDisplay()->getGfx();
    auto input = system->getInput();

    // One-shot snapshot of all debounced inputs (maintained by InputHandler).
    uint16_t currentState = input->getDebouncedStates();

    // Pure calculation: no RAM access, only internal CPU registers and ALU
    uint16_t changed = currentState ^ lastState;

    if (changed != 0) {
        // Capture the HAL-side edge timestamp once, BEFORE drawing. The delta
        // at the end measures the full path: edge detected in InputHandler ->
        // bitmap published -> read here -> redraw finished. 
        const unsigned long edgeMs = input->getLastEdgeMs();

        // Joystick (any direction changed -> redraw the whole stick)
        if (changed & JOY_MASK) {
            drawJoystick(gfx, 35, 80,
                currentState & EVENT_BIT(ControlEvent::JOY_UP),
                currentState & EVENT_BIT(ControlEvent::JOY_DOWN),
                currentState & EVENT_BIT(ControlEvent::JOY_LEFT),
                currentState & EVENT_BIT(ControlEvent::JOY_RIGHT));
        }

        // Arcade Buttons (A, B, X, Y)
        if (changed & EVENT_BIT(ControlEvent::BTN_A)) drawArcadeBtn(gfx, 75,  85, 9, Colors::RED, currentState & EVENT_BIT(ControlEvent::BTN_A));
        if (changed & EVENT_BIT(ControlEvent::BTN_B)) drawArcadeBtn(gfx, 97,  73, 9, Colors::YELLOW, currentState & EVENT_BIT(ControlEvent::BTN_B));
        if (changed & EVENT_BIT(ControlEvent::BTN_X)) drawArcadeBtn(gfx, 121, 73, 9, Colors::GREEN, currentState & EVENT_BIT(ControlEvent::BTN_X));
        if (changed & EVENT_BIT(ControlEvent::BTN_Y)) drawArcadeBtn(gfx, 143, 85, 9, Colors::CYAN, currentState & EVENT_BIT(ControlEvent::BTN_Y));

        // Shoulder Buttons (L2, L1, R1, R2)
        if (changed & EVENT_BIT(ControlEvent::BTN_L2)) drawArcadeBtn(gfx, 75,  60, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_L2));
        if (changed & EVENT_BIT(ControlEvent::BTN_L1)) drawArcadeBtn(gfx, 97,  48, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_L1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R1)) drawArcadeBtn(gfx, 121, 48, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_R1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R2)) drawArcadeBtn(gfx, 143, 60, 8, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_R2));

        // Start & Select
        if (changed & EVENT_BIT(ControlEvent::BTN_START))  drawArcadeBtn(gfx, 15, 40, 5, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_START));
        if (changed & EVENT_BIT(ControlEvent::BTN_SELECT)) drawArcadeBtn(gfx, 30, 40, 5, Colors::BLACK, currentState & EVENT_BIT(ControlEvent::BTN_SELECT));

        lastState = currentState;

        // Diagnostic line: real edge-to-screen latency (LAT) plus the
        // duration of the last completed press (DUR). DUR persists between
        // releases so it always shows the most recent hold time.
        gfx->setTextColor(Colors::GREEN, Colors::BLACK);
        gfx->setCursor(2, 16);
        gfx->printf("LAT: %lu ms  DUR: %lu ms ",
                    millis() - edgeMs,
                    input->getLastReleasedDurationMs());
    }

    // --- EXIT LOGIC: SELECT + L2 + R2 ---
    
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);

    // Combo logic: Only if all three are currently pressed
    if (durSel > 0 && durL2 > 0 && durR2 > 0) {
        
        // The "shared" hold time is the minimum duration among the three buttons
        unsigned long comboTime = min(durSel, min(durL2, durR2));

        // Use the centralized progress bar (drawn in red)
        system->getDisplay()->drawProgressBar(comboTime, 2000, Colors::RED);

        // 2-Second Check
        if (comboTime >= 2000) {
            system->getDisplay()->clearProgressBar();
            system->getAppManager()->startApp(AppId::Menu);
            return;
        }
    } else {
        // Clear the bar if any of the three buttons is released
        system->getDisplay()->clearProgressBar();
    }
}

void InputMonitorApp::drawJoystick(TFT_eSPI* gfx, int baseX, int baseY, bool up, bool down, bool left, bool right) {
    TFT_eSprite spr = TFT_eSprite(gfx);
    spr.createSprite(50, 50); 
    
    int cx = 25; 
    int cy = 25;
    
    spr.fillSprite(Colors::BLACK); 

    // Base ring: Black fill, WHITE outline
    spr.fillCircle(cx, cy, 16, Colors::BLACK); 
    spr.drawCircle(cx, cy, 16, Colors::WHITE); 

    // Knob position
    int kx = cx;
    int ky = cy;
    bool isDiagonal = (up || down) && (left || right);
    int step = isDiagonal ? 11 : 16;
    
    if (up)    ky -= step;
    if (down)  ky += step;
    if (left)  kx -= step;
    if (right) kx += step;
    
    bool pressed = (up || down || left || right);
    
    // Shaft & Knob: ALWAYS WHITE
    if (pressed) {
        spr.drawLine(cx, cy, kx, ky, Colors::WHITE); 
        spr.drawLine(cx-1, cy, kx, ky, Colors::WHITE); 
    }
    spr.fillCircle(kx, ky, 8, Colors::WHITE); 

    spr.pushSprite(baseX - 25, baseY - 25);
    spr.deleteSprite();
}

void InputMonitorApp::drawArcadeBtn(TFT_eSPI* gfx, int x, int y, int r, uint16_t color, bool pressed) {
    TFT_eSprite spr = TFT_eSprite(gfx);
    int size = (r * 2) + 2; 
    spr.createSprite(size, size);
    
    int cx = size / 2;
    int cy = size / 2;
    spr.fillSprite(Colors::BLACK); 
    
    if (pressed) {
        // When pressed, all buttons turn white
        spr.fillCircle(cx, cy, r, Colors::WHITE); 
    } else {
        if (color == Colors::BLACK) {
            // System buttons remain BLACK
            spr.fillCircle(cx, cy, r, Colors::BLACK);
        } else {
            // Colored buttons keep their assigned COLOR
            spr.fillCircle(cx, cy, r, color);
        }
    }
    
    // OUTLINE ALWAYS WHITE
    spr.drawCircle(cx, cy, r, Colors::WHITE); 
    
    spr.pushSprite(x - cx, y - cy);
    spr.deleteSprite();
}