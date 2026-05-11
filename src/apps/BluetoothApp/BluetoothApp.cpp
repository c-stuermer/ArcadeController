/**
 * Project: Arcade Controller V1.0
 * File: apps/BluetoothApp/BluetoothApp.cpp
 * Description: Owns the gamepad transport (BLE-HID; Switch 2 planned) and
 *              provides the local Bluetooth UI / state machine that the
 *              V0.2 build had: IDLE -> PAIRING -> CONNECTED -> PAUSED.
 */

#include "BluetoothApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../AppId.h"
#include "../../hal/SettingsManager.h"
#include "../../hal/PowerManager.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/InputHandler.h"
#include <NimBLEDevice.h>

BluetoothApp::BluetoothApp(ISystem* sys) : App(sys) {}

// --- Lifecycle --------------------------------------------------------------

void BluetoothApp::start() {
    Serial.println("[APP] BluetoothApp starting...");

    // First entry: load persisted mode and bring up the adapter.
    if (!adapterInitialized) {
        uint8_t raw  = system->getSettings()->getGamepadMode();
        Mode    mode = (raw == static_cast<uint8_t>(Mode::SWITCH))
                         ? Mode::SWITCH
                         : Mode::BLE_HID;
        applyMode(mode, /*persist=*/false);
        adapterInitialized = true;
    }

    // Determine entry state from current connection status (V0.2 behaviour).
    if (active && active->isConnected()) {
        currentState = BtState::CONNECTED;
    } else {
        currentState = BtState::IDLE;
        if (active) active->stopAdvertising();
    }
    lastDrawnState = BtState::PAUSED;  // force redraw on next update()
}

void BluetoothApp::update() {
    auto input = system->getInput();
    auto disp  = system->getDisplay();

    // Periodic battery push to the active gamepad
    if (active && (millis() - lastBatterySync >= BATTERY_SYNC_MS || lastBatterySync == 0)) {
        active->setBatteryLevel(system->getPower()->getBatteryPercentage());
        lastBatterySync = millis();
    }

    // 1. Automatic state transitions
    if (active) {
        if (currentState == BtState::PAIRING && active->isConnected()) {
            currentState = BtState::CONNECTED;
        } else if (currentState == BtState::CONNECTED && !active->isConnected()) {
            currentState = BtState::IDLE;
            active->stopAdvertising();
        }
    }

    // 2. The 2-second emergency exit combo (SELECT + L2 + R2)
    unsigned long durSel = input->getDuration(ControlEvent::BTN_SELECT);
    unsigned long durL2  = input->getDuration(ControlEvent::BTN_L2);
    unsigned long durR2  = input->getDuration(ControlEvent::BTN_R2);

    if (durSel > 0 && durL2 > 0 && durR2 > 0) {
        unsigned long comboTime = min(durSel, min(durL2, durR2));

        // Draw progress bar (Red for "Exit")
        disp->drawProgressBar(comboTime, 2000, 0xF800);

        if (comboTime >= 2000) {
            disp->clearProgressBar();

            // Virtually release combo buttons so the host doesn't see them
            // as stuck after we leave passthrough.
            if (active) {
                active->release(ControlEvent::BTN_SELECT);
                active->release(ControlEvent::BTN_L2);
                active->release(ControlEvent::BTN_R2);
            }

            // Wake hardware if it was killed by stealth boot
            if (disp->getBrightness() == 0) {
                Serial.println("[BLUETOOTH] Waking up hardware...");
                system->setBrightness(system->getSettings()->getBrightness());
                system->setVolume(system->getSettings()->getVolume());
            }

            Serial.println("[BLUETOOTH] Escaping to PAUSED menu...");
            currentState = BtState::PAUSED;
        }
    } else {
        disp->clearProgressBar();
    }

    // 3. Render only on state change (prevents flicker)
    if (currentState != lastDrawnState) {
        drawScreen();
        lastDrawnState = currentState;
    }
}

void BluetoothApp::stop() {
    // Intentionally NOT tearing down the adapter so the gamepad keeps
    // working when the user switches to another app (e.g. the menu).
    // Just clear the progress bar so it doesn't bleed into the next app.
    system->getDisplay()->clearProgressBar();
}

void BluetoothApp::onInput(ControlEvent ev, EventType type) {
    if (!active) return;

    // Ignore non-press events unless we are actively passing through
    if (type != EventType::PRESSED && currentState != BtState::CONNECTED) return;

    // --- STATE: CONNECTED (passthrough to host) ---
    if (currentState == BtState::CONNECTED) {
        if (type == EventType::PRESSED)       active->press(ev);
        else if (type == EventType::RELEASED) active->release(ev);
        return;
    }

    // --- STATE: IDLE (BT sleeping) ---
    if (currentState == BtState::IDLE) {
        if (ev == ControlEvent::BTN_B) {
            system->getAppManager()->startApp(AppId::Menu);
        } else if (ev == ControlEvent::BTN_A) {
            currentState = BtState::PAIRING;
            active->startAdvertising();
        } else if (ev == ControlEvent::BTN_X) {
            NimBLEDevice::deleteAllBonds();
        }
    }
    // --- STATE: PAIRING (waiting for host) ---
    else if (currentState == BtState::PAIRING) {
        if (ev == ControlEvent::BTN_B) {
            currentState = BtState::IDLE;
            active->stopAdvertising();
        }
    }
    // --- STATE: PAUSED (local overlay menu) ---
    else if (currentState == BtState::PAUSED) {
        if (ev == ControlEvent::BTN_B) {
            currentState = BtState::CONNECTED;  // Back to game
        } else if (ev == ControlEvent::BTN_A) {
            // Full stop: disconnect AND stop broadcasting
            active->disconnect();
            active->stopAdvertising();
            currentState = BtState::IDLE;
        } else if (ev == ControlEvent::BTN_START) {
            // Quit to main menu but stay connected in the background
            system->getAppManager()->startApp(AppId::Menu);
        }
    }
}

// --- Mode management --------------------------------------------------------

void BluetoothApp::switchMode(Mode newMode) {
    if (newMode == currentMode && active != nullptr) return;
    applyMode(newMode, /*persist=*/true);
    lastDrawnState = BtState::PAUSED;  // force redraw
}

IGamepadOutput* BluetoothApp::adapterFor(Mode mode) {
    switch (mode) {
        case Mode::BLE_HID: return &bleAdapter;
        case Mode::SWITCH:  /* return &switchAdapter; */ return &bleAdapter; // fallback until Switch2 is implemented
    }
    return &bleAdapter;
}

void BluetoothApp::applyMode(Mode mode, bool persist) {
    // Tear down previous adapter (if any)
    if (active) {
        active->stopAdvertising();
        active->disconnect();
    }

    // Bring up the new one
    currentMode = mode;
    active      = adapterFor(mode);
    active->begin();
    active->startAdvertising();

    // Push current battery state immediately so the host shows it
    active->setBatteryLevel(system->getPower()->getBatteryPercentage());
    lastBatterySync = millis();

    if (persist) {
        system->getSettings()->setGamepadMode(static_cast<uint8_t>(mode));
    }
}

// --- UI ---------------------------------------------------------------------

void BluetoothApp::drawScreen() {
    auto disp = system->getDisplay();
    auto gfx  = disp->getGfx();

    gfx->fillScreen(0x0000);
    disp->drawHeader("BLUETOOTH");

    // UI Layout Variables (no horizontal lines, V0.2 spacing)
    int titleY      = 26;
    int textY       = 50;
    int lineSpacing = 18;

    gfx->setTextSize(1);

    if (currentState == BtState::IDLE) {
        gfx->setTextColor(0x7BEF);
        gfx->drawString("STANDBY", 10, titleY);

        gfx->setTextColor(0xFFFF);
        gfx->drawString("[A] Connect / Search", 10, textY);
        gfx->drawString("[X] Clear old Bonds", 10, textY + lineSpacing);

        gfx->setTextColor(0xF800);  // Red exit hint
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
        gfx->setTextColor(0xFFFF);
        gfx->drawString("HOST: " + peerMac, 10, textY);

        // Combo Info
        gfx->setTextColor(0xF800);
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
        gfx->setTextColor(0x07E0);  // Green for Resume
        gfx->drawString("[B] Resume Game", 10, textY + lineSpacing + 4);

        gfx->setTextColor(0xFFFF);
        gfx->drawString("[A] Disconnect", 10, textY + (lineSpacing * 2) + 4);
        gfx->drawString("[START] Quit to Menu", 10, textY + (lineSpacing * 3) + 4);
    }
}
