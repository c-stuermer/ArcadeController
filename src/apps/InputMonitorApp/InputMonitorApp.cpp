/**
 * Project: Arcade Controller V1.0
 * File: InputMonitorApp.cpp
 * Description: Implementation of the input monitor UI and logic.
 */

#include "InputMonitorApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/InputHandler.h"

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

    // Bit-packer: Write all current inputs into a single register
    uint16_t currentState = 0;
    if (input->isPressed(ControlEvent::JOY_UP))    currentState |= (1 << 0);
    if (input->isPressed(ControlEvent::JOY_DOWN))  currentState |= (1 << 1);
    if (input->isPressed(ControlEvent::JOY_LEFT))  currentState |= (1 << 2);
    if (input->isPressed(ControlEvent::JOY_RIGHT)) currentState |= (1 << 3);
    if (input->isPressed(ControlEvent::BTN_A))     currentState |= (1 << 4);
    if (input->isPressed(ControlEvent::BTN_B))     currentState |= (1 << 5);
    if (input->isPressed(ControlEvent::BTN_X))     currentState |= (1 << 6);
    if (input->isPressed(ControlEvent::BTN_Y))     currentState |= (1 << 7);
    if (input->isPressed(ControlEvent::BTN_L2))    currentState |= (1 << 8);
    if (input->isPressed(ControlEvent::BTN_L1))    currentState |= (1 << 9);
    if (input->isPressed(ControlEvent::BTN_R1))    currentState |= (1 << 10);
    if (input->isPressed(ControlEvent::BTN_R2))    currentState |= (1 << 11);
    if (input->isPressed(ControlEvent::BTN_START)) currentState |= (1 << 12);
    if (input->isPressed(ControlEvent::BTN_SELECT))currentState |= (1 << 13);

    // Pure calculation: no RAM access, only internal CPU registers and ALU
    uint16_t changed = currentState ^ lastState;    

    if (changed != 0) {
        unsigned long startTime = millis();

        // Efficient bit query for graphic updates
        if (changed & 0x000F) { // Joystick-Bits 0-3
            drawJoystick(gfx, 35, 80, (currentState & (1<<0)), (currentState & (1<<1)), 
                                      (currentState & (1<<2)), (currentState & (1<<3)));
        }

        // Arcade Buttons (A, B, X, Y)
        if (changed & (1<<4)) drawArcadeBtn(gfx, 75, 85, 9, 0xF800, (currentState & (1<<4)));
        if (changed & (1<<5)) drawArcadeBtn(gfx, 97, 73, 9, 0xFFE0, (currentState & (1<<5)));
        if (changed & (1<<6)) drawArcadeBtn(gfx, 121, 73, 9, 0x07E0, (currentState & (1<<6)));
        if (changed & (1<<7)) drawArcadeBtn(gfx, 143, 85, 9, 0x07FF, (currentState & (1<<7)));

        // System Buttons (L, R)
        if (changed & (1<<8))  drawArcadeBtn(gfx, 75, 60, 8, 0x0000, (currentState & (1<<8)));
        if (changed & (1<<9))  drawArcadeBtn(gfx, 97, 48, 8, 0x0000, (currentState & (1<<9)));
        if (changed & (1<<10)) drawArcadeBtn(gfx, 121, 48, 8, 0x0000, (currentState & (1<<10)));
        if (changed & (1<<11)) drawArcadeBtn(gfx, 143, 60, 8, 0x0000, (currentState & (1<<11)));

        // Start & Select
        if (changed & (1<<12)) drawArcadeBtn(gfx, 15, 40, 5, 0x0000, (currentState & (1<<12)));
        if (changed & (1<<13)) drawArcadeBtn(gfx, 30, 40, 5, 0x0000, (currentState & (1<<13)));

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