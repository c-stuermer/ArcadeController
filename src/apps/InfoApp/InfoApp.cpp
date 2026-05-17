/**
 * Project: Arcade Controller V1.2
 * File: InfoApp.cpp
 * Description: Implementation of the system information screen.
 */

#include "InfoApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../../config/Colors.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/PowerManager.h"
#include <esp_mac.h>

void InfoApp::start() {
    Serial.println("[APP] InfoApp starting...");
    //clear screen, draw screen first time, reset update interval
    system->getDisplay()->getGfx()->fillScreen(Colors::BLACK);
    drawScreen();
    lastUpdate = millis();
}

void InfoApp::stop() {
    // Nothing special needed on exit
}

void InfoApp::update() {
    // Live update every 1000 milliseconds (1 second)
    if (millis() - lastUpdate > 1000) {
        drawScreen();
        lastUpdate = millis();
    }
}

void InfoApp::onInput(ControlEvent ev, EventType type) {
    // Exit the app on ANY button press
    if (type == EventType::PRESSED) {
        system->getAppManager()->startApp(AppId::Menu);
    }
}

void InfoApp::drawScreen() {
    auto* disp  = system->getDisplay();
    auto* gfx   = disp->getGfx();
    auto* power = system->getPower();

    // Alignment / layout
    const int leftCol  = 10;
    const int rightCol = 80;
    const int step     = 14;
    int       y        = 26;

    // Wipe the content area below the header, then redraw header.
    gfx->fillRect(0, 14, 160, 114, Colors::BLACK);
    disp->drawHeader("SYSTEM INFO");

    gfx->setTextSize(1);

    // --- SOFTWARE ---
    gfx->setTextColor(Colors::WHITE);
    gfx->drawString(FIRMWARE_VERSION, rightCol, y);
    y += step;

    // --- HARDWARE ---
    y += 4;
    gfx->setTextColor(Colors::GREY);
    gfx->drawString("CPU FREQ:", leftCol, y);
    gfx->setTextColor(Colors::GREEN);
    gfx->drawString(String(ESP.getCpuFreqMHz()) + " MHz", rightCol, y);
    y += step;

    gfx->setTextColor(Colors::GREY);
    gfx->drawString("FREE RAM:", leftCol, y);
    gfx->setTextColor(Colors::YELLOW);
    gfx->drawString(String(ESP.getFreeHeap() / 1024) + " KB", rightCol, y);
    y += step;

    // --- BATTERY & POWER ---
    y += 4;
    const float voltage = power->getBatteryVoltage();
    const int   percent = power->getBatteryPercentage();

    gfx->setTextColor(Colors::GREY);
    gfx->drawString("BATT VOLT:", leftCol, y);
    gfx->setTextColor(Colors::CYAN);
    gfx->drawString(String(voltage, 2) + " V", rightCol, y);
    y += step;

    gfx->setTextColor(Colors::GREY);
    gfx->drawString("BATT %:", leftCol, y);
    const uint16_t battColor = (percent > 20) ? Colors::GREEN : Colors::RED;
    gfx->setTextColor(battColor);
    gfx->drawString(String(percent) + " %", rightCol, y);
    y += step;

    // --- LOCAL MAC ADDRESS (Bottom) ---
    y += 4;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    gfx->setTextColor(Colors::GREY);
    gfx->drawString("MAC:", leftCol, y);
    gfx->setTextColor(Colors::WHITE);
    gfx->drawString(macStr, rightCol - 30, y);
}