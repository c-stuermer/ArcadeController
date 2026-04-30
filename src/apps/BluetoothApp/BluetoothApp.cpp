/**
 * Project: Arcade Controller V0.2
 * File: BluetoothApp.cpp
 * Description: Implementation of the Bluetooth UI and input routing.
 */

#include "BluetoothApp.h"
#include "../../ArcadeController.h" 
#include <NimBLEDevice.h> 

void BluetoothApp::start() {
    auto gamepad = system->getGamepad();
    
    if (gamepad->isConnected()) {
        currentState = BtState::CONNECTED;
    } else {
        currentState = BtState::IDLE;
        gamepad->stopAdvertising(); 
    }
    lastDrawnState = BtState::PAUSED; 
}

void BluetoothApp::stop() {
    // Ensure the progress bar is cleared when exiting
    system->getDisplay()->clearProgressBar(); 
}

void BluetoothApp::update() {
    auto gamepad = system->getGamepad();
    auto input = system->getInput();
    auto disp = system->getDisplay();

    // 1. Automatic state transitions
    if (currentState == BtState::PAIRING && gamepad->isConnected()) {
        currentState = BtState::CONNECTED;
    } else if (currentState == BtState::CONNECTED && !gamepad->isConnected()) {
        currentState = BtState::IDLE; 
        gamepad->stopAdvertising();
    }

    // 2. The 2-second emergency exit timer (SELECT + L2 + R2)
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);

    if (durSel > 0 && durL2 > 0 && durR2 > 0) {
        // The shortest hold duration among the 3 buttons is our combo time
        unsigned long comboTime = min(durSel, min(durL2, durR2));

        // Draw progress bar (Red for "Exit")
        disp->drawProgressBar(comboTime, 2000, 0xF800); 

        if (comboTime >= 2000) {
            disp->clearProgressBar();
            
            // Virtually release buttons to prevent stuck inputs on the host
            gamepad->release(ControlEvent::BTN_SELECT);
            gamepad->release(ControlEvent::BTN_L2);
            gamepad->release(ControlEvent::BTN_R2);

            if (disp->getBrightness() == 0) {
                Serial.println("[BLUETOOTH] Waking up hardware...");
                system->applySavedSettings(); 
            }
            
            Serial.println("[BLUETOOTH] Escaping to PAUSED menu...");
            currentState = BtState::PAUSED; 
        }
    } else {
        disp->clearProgressBar(); 
    }

    // 3. Render logic
    if (currentState != lastDrawnState) {
        drawScreen();
        lastDrawnState = currentState;
    }
}

void BluetoothApp::onInput(ControlEvent ev, EventType type) {
    auto gamepad = system->getGamepad();

    // Ignore non-press events unless we are actively connected to a host
    if (type != EventType::PRESSED && currentState != BtState::CONNECTED) return;

    // --- STATE: CONNECTED (Passthrough to PC/Console) ---
    if (currentState == BtState::CONNECTED) {
        if (type == EventType::PRESSED) gamepad->press(ev);
        else if (type == EventType::RELEASED) gamepad->release(ev);
        return; 
    }

    // --- STATE: IDLE (Bluetooth is sleeping) ---
    if (currentState == BtState::IDLE) {
        if (ev == ControlEvent::BTN_B) {
            system->startApp(system->getMenuApp());
        } else if (ev == ControlEvent::BTN_A) {
            // Start searching for a connection
            currentState = BtState::PAIRING;
            gamepad->startAdvertising(); 
        } else if (ev == ControlEvent::BTN_X) {
            // Clear saved bonds
            NimBLEDevice::deleteAllBonds();
        }
    }
    // --- STATE: PAIRING (Waiting for connection) ---
    else if (currentState == BtState::PAIRING) {
        if (ev == ControlEvent::BTN_B) {
            // Cancel search
            currentState = BtState::IDLE;
            gamepad->stopAdvertising();
        }
    }
    // --- STATE: PAUSED (Local overlay menu) ---
    else if (currentState == BtState::PAUSED) {
        if (ev == ControlEvent::BTN_B) {
            currentState = BtState::CONNECTED; // Back to game
        } else if (ev == ControlEvent::BTN_A) {
            // Full stop: Disconnect AND stop broadcasting
            gamepad->disconnect(); 
            gamepad->stopAdvertising();
            currentState = BtState::IDLE; 
        } else if (ev == ControlEvent::BTN_START) {
            // Quit to main menu but stay connected in the background
            system->startApp(system->getMenuApp()); 
        }
    }
}

void BluetoothApp::drawScreen() {
    auto disp = system->getDisplay();
    auto gfx = disp->getGfx();
    
    gfx->fillScreen(0x0000); 
    disp->drawHeader("BLUETOOTH");

    // UI Layout Variables (adjusted without horizontal lines)
    int titleY = 26;
    int textY = 50; 
    int lineSpacing = 18;

    gfx->setTextSize(1);

    if (currentState == BtState::IDLE) {
        gfx->setTextColor(0x7BEF); 
        gfx->drawString("STANDBY", 10, titleY);

        gfx->setTextColor(0xFFFF); 
        gfx->drawString("[A] Connect / Search", 10, textY);
        gfx->drawString("[X] Clear old Bonds", 10, textY + lineSpacing);
        
        gfx->setTextColor(0xF800); // Red exit hint at the bottom
        gfx->drawString("[B] Exit to Menu", 10, 105);

    } else if (currentState == BtState::PAIRING) {
        gfx->setTextColor(0xFFE0); 
        gfx->drawString("SEARCHING...", 10, titleY);

        gfx->setTextColor(0xFFFF);
        gfx->drawString("Make sure Pi/PC", 10, textY);
        gfx->drawString("is ready to pair.", 10, textY + 12);

        gfx->setTextColor(0x7BEF);
        gfx->drawString("[B] Cancel", 10, 105);

    } else if (currentState == BtState::CONNECTED) {
        gfx->setTextColor(0x07E0); 
        gfx->drawString("CONNECTED", 10, titleY);

        // --- Read HOST MAC Address ---
        String peerMac = "UNKNOWN";
        auto server = NimBLEDevice::getServer();
        if (server && server->getConnectedCount() > 0) {
            peerMac = String(server->getPeerInfo(0).getAddress().toString().c_str());
            peerMac.toUpperCase();
        }
        gfx->setTextColor(0xFFFF); // White for better readability
        gfx->drawString("HOST: " + peerMac, 10, textY);

        // Combo Info
        gfx->setTextColor(0xF800); // Red
        gfx->drawString("[SELECT] + [L2] + [R2]", 10, 105);

    } else if (currentState == BtState::PAUSED) {
        gfx->setTextColor(0xF800); 
        gfx->drawString("SYSTEM PAUSED", 10, titleY);

        // --- Read HOST MAC Address ---
        String peerMac = "UNKNOWN";
        auto server = NimBLEDevice::getServer();
        if (server && server->getConnectedCount() > 0) {
            peerMac = String(server->getPeerInfo(0).getAddress().toString().c_str());
            peerMac.toUpperCase();
        }
        gfx->setTextColor(0xFFFF);
        gfx->drawString("HOST: " + peerMac, 10, textY);

        // Buttons
        gfx->setTextColor(0x07E0); // Green for Resume
        gfx->drawString("[B] Resume Game", 10, textY + lineSpacing + 4);
        
        gfx->setTextColor(0xFFFF); // White for the rest
        gfx->drawString("[A] Disconnect", 10, textY + (lineSpacing * 2) + 4);
        gfx->drawString("[START] Quit to Menu", 10, textY + (lineSpacing * 3) + 4);
    }
}