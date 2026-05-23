/**
 * Project: Arcade Controller V1.3
 * File: InfoApp.cpp
 * Description: Implementation of the system information screen.
 */

#include "InfoApp.h"
#include "../../config/Colors.h"
#include <esp_mac.h>

InfoApp::InfoApp(IDisplay* display, IPower* power, IAppNavigator* appNavigator)
    : App(),
      display(display),
      power(power),
      appManager(appNavigator) {}

void InfoApp::start() {
    Serial.println("[APP] InfoApp starting...");
    display->clear();
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
        appManager->switchApp(AppId::Menu);
    }
}

void InfoApp::drawScreen() {
    // Alignment / layout
    const int leftCol  = 10;
    const int rightCol = 80;
    const int step     = 14;
    int       y        = 26;

    // Wipe the content area below the header, then redraw header.
    display->fillRect(0, 14, 160, 114, Colors::BLACK);
    display->drawHeader("SYSTEM INFO");

    // --- SOFTWARE ---
    display->drawText(rightCol, y, FIRMWARE_VERSION, Colors::WHITE);
    y += step;

    // --- HARDWARE ---
    y += 4;
    display->drawText(leftCol,  y, "CPU FREQ:",                            Colors::GREY);
    display->drawText(rightCol, y, String(ESP.getCpuFreqMHz()) + " MHz",   Colors::GREEN);
    y += step;

    display->drawText(leftCol,  y, "FREE RAM:",                                Colors::GREY);
    display->drawText(rightCol, y, String(ESP.getFreeHeap() / 1024) + " KB",   Colors::YELLOW);
    y += step;

    // --- BATTERY & POWER ---
    y += 4;
    const float voltage = power->getBatteryVoltage();
    const int   percent = power->getBatteryPercentage();

    display->drawText(leftCol,  y, "BATT VOLT:",                  Colors::GREY);
    display->drawText(rightCol, y, String(voltage, 2) + " V",     Colors::CYAN);
    y += step;

    const uint16_t battColor = (percent > 20) ? Colors::GREEN : Colors::RED;
    display->drawText(leftCol,  y, "BATT %:",                  Colors::GREY);
    display->drawText(rightCol, y, String(percent) + " %",     battColor);
    y += step;

    // --- LOCAL MAC ADDRESS (Bottom) ---
    y += 4;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    display->drawText(leftCol,      y, "MAC:",   Colors::GREY);
    display->drawText(rightCol - 30, y, macStr,  Colors::WHITE);

    display->flush();
}
