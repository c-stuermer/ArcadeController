/**
 * Project: Arcade Controller V0.1
 * File: InputMonitorApp.cpp
 * Description: Implementation of the input visualizer.
 */

#include "InputMonitorApp.h"
// Here we include the full controller definition to access hardware methods
#include "../../ArcadeController.h" 

void InputMonitorApp::start() {
    Serial.println("[App] InputMonitor started");
}

void InputMonitorApp::update() {
    static int heartbeat = 0;
    heartbeat++;

    // Get access to hardware subsystems
    U8G2* gfx = system->getDisplay()->getGfx();
    InputHandler* input = system->getInput(); 
    
    // 1. Draw Header
    system->getDisplay()->drawHeader("INPUT CHECK", system->getPower()->getBatteryPercentage(), system->getPower()->isUSBConnected());

    // 2. Draw Buttons (Visual Layout)
    
    // Select & Start (Top Left)
    drawBtn(gfx, 15, 25, 4, "St",  input->isPressed(ControlEvent::BTN_START));
    drawBtn(gfx, 35, 25, 4, "S",   input->isPressed(ControlEvent::BTN_SELECT));

    // Joystick (Visual Cross)
    int joyX = 25, joyY = 48, r = 5;
    drawBtn(gfx, joyX, joyY - 8, r, "U", input->isPressed(ControlEvent::JOY_UP));
    drawBtn(gfx, joyX, joyY + 8, r, "D", input->isPressed(ControlEvent::JOY_DOWN));
    drawBtn(gfx, joyX - 8, joyY, r, "L", input->isPressed(ControlEvent::JOY_LEFT));
    drawBtn(gfx, joyX + 8, joyY, r, "R", input->isPressed(ControlEvent::JOY_RIGHT));

    // Arcade Buttons (2x4 Grid)
    int gridX = 60;
    int rowTop = 28; // L/R Row
    int rowBot = 48; // A/B/X/Y Row
    int sp = 17;     // Spacing
    int rb = 7;      // Radius (Big buttons)

    // Top Row: L2 L1 R1 R2
    drawBtn(gfx, gridX + 0*sp, rowTop, rb, "L2", input->isPressed(ControlEvent::BTN_L2));
    drawBtn(gfx, gridX + 1*sp, rowTop, rb, "L1", input->isPressed(ControlEvent::BTN_L1));
    drawBtn(gfx, gridX + 2*sp, rowTop, rb, "R1", input->isPressed(ControlEvent::BTN_R1));
    drawBtn(gfx, gridX + 3*sp, rowTop, rb, "R2", input->isPressed(ControlEvent::BTN_R2));

    // Bottom Row: A B X Y
    drawBtn(gfx, gridX + 0*sp, rowBot, rb, "A", input->isPressed(ControlEvent::BTN_A));
    drawBtn(gfx, gridX + 1*sp, rowBot, rb, "B", input->isPressed(ControlEvent::BTN_B));
    drawBtn(gfx, gridX + 2*sp, rowBot, rb, "X", input->isPressed(ControlEvent::BTN_X));
    drawBtn(gfx, gridX + 3*sp, rowBot, rb, "Y", input->isPressed(ControlEvent::BTN_Y));

    // 3. Draw Frame Counter (to check for I2C lag)
    gfx->setDrawColor(1);
    gfx->setFont(u8g2_font_6x10_tr);
    gfx->setCursor(0, 60);
    gfx->print("FPS: ");
    gfx->print(heartbeat);
}

void InputMonitorApp::onInput(ControlEvent ev, EventType type) {
    // Debugging logic stays in Serial Monitor
    if(type == EventType::RELEASED) {
        InputHandler* input = system->getInput();
        unsigned long duration = input->getLastPressDuration(ev);
        
        Serial.printf("[Input] %s RELEASED (Held for %lu ms)\n", eventToString(ev), duration);
        
        // Optional: Feedback sound
        // system->getSound()->play(SoundEffect::CLICK);
    }
}

// Helper: Draws a filled circle if active, hollow if inactive
void InputMonitorApp::drawBtn(U8G2* gfx, int x, int y, int r, const char* label, bool active) {
    // 1. Clear background (Black filled circle)
    gfx->setDrawColor(0);
    gfx->drawDisc(x, y, r);
    
    // 2. Draw Button State
    if (active) {
        gfx->setDrawColor(1);   // White
        gfx->drawDisc(x, y, r); // Filled
        gfx->setDrawColor(0);   // Text Black on White
    } else {
        gfx->setDrawColor(1);     // White
        gfx->drawCircle(x, y, r); // Outline only
        // Text White (default)
    }
    
    // 3. Draw Label Centered
    gfx->setFont(u8g2_font_micro_tr); 
    int w = gfx->getStrWidth(label);
    gfx->setCursor(x - w / 2, y + 3); 
    gfx->print(label);
    
    gfx->setDrawColor(1); // Reset color for next operations
}