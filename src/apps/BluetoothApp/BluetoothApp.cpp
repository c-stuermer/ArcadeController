/**
 * Project: Arcade Controller V1.3
 * File: apps/BluetoothApp/BluetoothApp.cpp
 */

#include "BluetoothApp.h"
#include "../Interfaces/IAppNavigator.h"
#include "../../config/Colors.h"
#include <NimBLEDevice.h>

BluetoothApp::BluetoothApp(IDisplay*   display,
                           IPower*     power,
                           ISetting*   settings,
                           IInputH*    input,
                           IAppNavigator* appNavigator)
    : App(),
      display(display),
      power(power),
      settings(settings),
      input(input),
      appNavigator(appNavigator) {}

// --- Lifecycle --------------------------------------------------------------

void BluetoothApp::start() {
    Serial.println("[APP] BluetoothApp starting...");

    // First entry: load persisted mode and bring up the adapter.
    if (!adapterInitialized) {
        uint8_t raw  = settings->getGamepadMode();
        Mode    mode = (raw == static_cast<uint8_t>(Mode::SWITCH))
                         ? Mode::SWITCH
                         : Mode::BLE_HID;
        applyMode(mode, /*persist=*/false);
        adapterInitialized = true;
    }

    // Determine entry state from current connection status.
    if (active && active->isConnected()) {
        currentState = BtState::CONNECTED;
    } else {
        currentState = BtState::IDLE;
        if (active) active->stopAdvertising();
    }
    lastDrawnState = BtState::PAUSED;  // force redraw on next update()
}

void BluetoothApp::update() {
    // Periodic battery push to the active gamepad
    if (active && (millis() - lastBatterySync >= BATTERY_SYNC_MS || lastBatterySync == 0)) {
        active->setBatteryLevel(power->getBatteryPercentage());
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

        display->drawProgressBar(comboTime, 2000, Colors::RED);

        if (comboTime >= 2000) {
            display->clearProgressBar();

            // Virtually release combo buttons so the host doesn't see them
            // as stuck after we leave passthrough.
            if (active) {
                active->release(ControlEvent::BTN_SELECT);
                active->release(ControlEvent::BTN_L2);
                active->release(ControlEvent::BTN_R2);
            }

            // Wake hardware if it was killed by stealth boot. settings->setX
            // persists AND applies (observer) — passing the already-saved
            // value re-applies it to the hardware without changing flash.
            if (display->getBrightness() == 0) {
                Serial.println("[BLUETOOTH] Waking up hardware...");
                settings->setBrightness(settings->getBrightness());
                settings->setVolume(settings->getVolume());
            }

            Serial.println("[BLUETOOTH] Escaping to PAUSED menu...");
            currentState = BtState::PAUSED;
        }
    } else {
        display->clearProgressBar();
    }

    // 3. Render only on state change (prevents flicker)
    if (currentState != lastDrawnState) {
        drawScreen();
        lastDrawnState = currentState;
        display->flush();
    }
}

void BluetoothApp::stop() {
    // Intentionally NOT tearing down the adapter so the gamepad keeps
    // working when the user switches to another app (e.g. the menu).
    display->clearProgressBar();
}

void BluetoothApp::onInput(ControlEvent ev, EventType type) {
    if (!active) return;

    // Outside CONNECTED state, only the press-edge drives the UI -- otherwise
    // every button would fire twice (once on press, once on release).
    if (type != EventType::PRESSED && currentState != BtState::CONNECTED) return;

    switch (currentState) {

        // --- CONNECTED: passthrough to host ---
        case BtState::CONNECTED:
            if (type == EventType::PRESSED)       active->press(ev);
            else if (type == EventType::RELEASED) active->release(ev);
            break;

        // --- IDLE: BT sleeping, waiting for user to start pairing ---
        case BtState::IDLE:
            if (ev == ControlEvent::BTN_B) {
                appNavigator->closeApp();
            } else if (ev == ControlEvent::BTN_A) {
                currentState = BtState::PAIRING;
                active->startAdvertising();
            } else if (ev == ControlEvent::BTN_X) {
                NimBLEDevice::deleteAllBonds();
            }
            break;

        // --- PAIRING: advertising, waiting for host to connect ---
        case BtState::PAIRING:
            if (ev == ControlEvent::BTN_B) {
                currentState = BtState::IDLE;
                active->stopAdvertising();
            }
            break;

        // --- PAUSED: local overlay menu while a host is connected ---
        case BtState::PAUSED:
            if (ev == ControlEvent::BTN_B) {
                currentState = BtState::CONNECTED;          // Back to game
            } else if (ev == ControlEvent::BTN_A) {
                // Full stop: disconnect AND stop broadcasting
                active->disconnect();
                active->stopAdvertising();
                currentState = BtState::IDLE;
            } else if (ev == ControlEvent::BTN_START) {
                // Quit to main menu but stay connected in the background
                appNavigator->closeApp();
            }
            break;
    }
}

// --- Mode management --------------------------------------------------------

void BluetoothApp::switchMode(Mode newMode) {
    if (newMode == currentMode && active != nullptr) return;
    applyMode(newMode, /*persist=*/true);
    lastDrawnState = BtState::PAUSED;  // force redraw
}

GamepadOutput* BluetoothApp::adapterFor(Mode mode) {
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
    active->setBatteryLevel(power->getBatteryPercentage());
    lastBatterySync = millis();

    if (persist) {
        settings->setGamepadMode(static_cast<uint8_t>(mode));
    }
}

// --- UI ---------------------------------------------------------------------

void BluetoothApp::drawScreen() {
    display->clear();
    display->drawHeader("BLUETOOTH");

    // UI Layout
    const int titleY      = 26;
    const int textY       = 50;
    const int lineSpacing = 18;

    // Peer MAC is shown in both CONNECTED and PAUSED. drawScreen() is only
    // called on state change, so reading once up-front (even when unused) is
    // cheaper than two duplicated blocks below.
    String peerMac = "UNKNOWN";
    if (auto* server = NimBLEDevice::getServer()) {
        if (server->getConnectedCount() > 0) {
            peerMac = String(server->getPeerInfo(0).getAddress().toString().c_str());
            peerMac.toUpperCase();
        }
    }

    switch (currentState) {

        // --- IDLE: standby screen with pairing / bond-clear hints ---
        case BtState::IDLE:
            display->drawText(10, titleY,             "STANDBY",              Colors::GREY);
            display->drawText(10, textY,              "[A] Connect / Search", Colors::WHITE);
            display->drawText(10, textY + lineSpacing,"[X] Clear old Bonds",  Colors::WHITE);
            display->drawText(10, 105,                "[B] Exit to Menu",     Colors::RED);
            break;

        // --- PAIRING: searching for a host ---
        case BtState::PAIRING:
            display->drawText(10, titleY,    "SEARCHING...",     Colors::YELLOW);
            display->drawText(10, textY,     "Make sure Pi/PC",  Colors::WHITE);
            display->drawText(10, textY + 12,"is ready to pair.",Colors::WHITE);
            display->drawText(10, 105,       "[B] Cancel",       Colors::GREY);
            break;

        // --- CONNECTED: passthrough active, show host + exit combo ---
        case BtState::CONNECTED:
            display->drawText(10, titleY, "CONNECTED",                Colors::GREEN);
            display->drawText(10, textY,  "HOST: " + peerMac,         Colors::WHITE);
            display->drawText(10, 105,    "[SELECT] + [L2] + [R2]",   Colors::RED);
            break;

        // --- PAUSED: overlay menu while still connected ---
        case BtState::PAUSED:
            display->drawText(10, titleY,                        "SYSTEM PAUSED",       Colors::RED);
            display->drawText(10, textY,                         "HOST: " + peerMac,    Colors::WHITE);
            display->drawText(10, textY + lineSpacing + 4,       "[B] Resume Game",     Colors::GREEN);
            display->drawText(10, textY + (lineSpacing * 2) + 4, "[A] Disconnect",      Colors::WHITE);
            display->drawText(10, textY + (lineSpacing * 3) + 4, "[START] Quit to Menu",Colors::WHITE);
            break;
    }
}
