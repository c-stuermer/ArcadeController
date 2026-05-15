/**
 * Project: Arcade Controller V1.1
 * File: InputMonitorApp.cpp
 * Description: Implementation of the input monitor UI and logic.
 */

#include "InputMonitorApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
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
    gfx->fillScreen(0x0000);
    
    // --- Static text for exit combo (like in the Bluetooth App) ---
    gfx->setTextSize(1);
    gfx->setTextColor(0xF800); // Red
    gfx->drawString("[SELECT] + [L2] + [R2]", 10, 112); 

    lastState = 0xFFFF; // Forces a full redraw on first update
}

void InputMonitorApp::stop() {
    // Ensure the progress bar is cleared when exiting the app
    system->getDisplay()->clearProgressBar();
}

void InputMonitorApp::onInput(ControlEvent ev, EventType type) {
    // Inputs are handled directly via polling in update() for minimal latency
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
        unsigned long startTime = millis();

        // Joystick (any direction changed -> redraw the whole stick)
        if (changed & JOY_MASK) {
            drawJoystick(gfx, 35, 80,
                currentState & EVENT_BIT(ControlEvent::JOY_UP),
                currentState & EVENT_BIT(ControlEvent::JOY_DOWN),
                currentState & EVENT_BIT(ControlEvent::JOY_LEFT),
                currentState & EVENT_BIT(ControlEvent::JOY_RIGHT));
        }

        // Arcade Buttons (A, B, X, Y)
        if (changed & EVENT_BIT(ControlEvent::BTN_A)) drawArcadeBtn(gfx, 75,  85, 9, 0xF800, currentState & EVENT_BIT(ControlEvent::BTN_A));
        if (changed & EVENT_BIT(ControlEvent::BTN_B)) drawArcadeBtn(gfx, 97,  73, 9, 0xFFE0, currentState & EVENT_BIT(ControlEvent::BTN_B));
        if (changed & EVENT_BIT(ControlEvent::BTN_X)) drawArcadeBtn(gfx, 121, 73, 9, 0x07E0, currentState & EVENT_BIT(ControlEvent::BTN_X));
        if (changed & EVENT_BIT(ControlEvent::BTN_Y)) drawArcadeBtn(gfx, 143, 85, 9, 0x07FF, currentState & EVENT_BIT(ControlEvent::BTN_Y));

        // Shoulder Buttons (L2, L1, R1, R2)
        if (changed & EVENT_BIT(ControlEvent::BTN_L2)) drawArcadeBtn(gfx, 75,  60, 8, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_L2));
        if (changed & EVENT_BIT(ControlEvent::BTN_L1)) drawArcadeBtn(gfx, 97,  48, 8, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_L1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R1)) drawArcadeBtn(gfx, 121, 48, 8, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_R1));
        if (changed & EVENT_BIT(ControlEvent::BTN_R2)) drawArcadeBtn(gfx, 143, 60, 8, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_R2));

        // Start & Select
        if (changed & EVENT_BIT(ControlEvent::BTN_START))  drawArcadeBtn(gfx, 15, 40, 5, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_START));
        if (changed & EVENT_BIT(ControlEvent::BTN_SELECT)) drawArcadeBtn(gfx, 30, 40, 5, 0x0000, currentState & EVENT_BIT(ControlEvent::BTN_SELECT));

        lastState = currentState;

        // Performance measurement
        gfx->setTextColor(0x07E0, 0x0000);
        gfx->setCursor(2, 16);
        gfx->printf("LATENCY: %lu ms ", millis() - startTime);
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
        system->getDisplay()->drawProgressBar(comboTime, 2000, 0xF800);

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
    
    spr.fillSprite(0x0000); 

    // Base ring: Black fill, WHITE outline
    spr.fillCircle(cx, cy, 16, 0x0000); 
    spr.drawCircle(cx, cy, 16, 0xFFFF); 

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
        spr.drawLine(cx, cy, kx, ky, 0xFFFF); 
        spr.drawLine(cx-1, cy, kx, ky, 0xFFFF); 
    }
    spr.fillCircle(kx, ky, 8, 0xFFFF); 

    spr.pushSprite(baseX - 25, baseY - 25);
    spr.deleteSprite();
}

void InputMonitorApp::drawArcadeBtn(TFT_eSPI* gfx, int x, int y, int r, uint16_t color, bool pressed) {
    TFT_eSprite spr = TFT_eSprite(gfx);
    int size = (r * 2) + 2; 
    spr.createSprite(size, size);
    
    int cx = size / 2;
    int cy = size / 2;
    spr.fillSprite(0x0000); 
    
    if (pressed) {
        // When pressed, all buttons turn white
        spr.fillCircle(cx, cy, r, 0xFFFF); 
    } else {
        if (color == 0x0000) {
            // System buttons remain BLACK
            spr.fillCircle(cx, cy, r, 0x0000);
        } else {
            // Colored buttons keep their assigned COLOR
            spr.fillCircle(cx, cy, r, color);
        }
    }
    
    // OUTLINE ALWAYS WHITE
    spr.drawCircle(cx, cy, r, 0xFFFF); 
    
    spr.pushSprite(x - cx, y - cy);
    spr.deleteSprite();
}