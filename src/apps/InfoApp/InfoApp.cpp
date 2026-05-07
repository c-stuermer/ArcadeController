/**
 * Project: Arcade Controller V0.2
 * File: InfoApp.cpp
 * Description: Implementation of the system information screen.
 */

#include "InfoApp.h"
#include "../../ArcadeController.h"
#include <esp_mac.h> 

void InfoApp::start() {
    //clear screen, draw screen first time, reset update interval
    system->getDisplay()->getGfx()->fillScreen(0x0000);
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
        system->startApp(system->getMenuApp()); 
    }
}

void InfoApp::drawScreen() {
    auto disp = system->getDisplay();
    auto gfx = disp->getGfx();
    auto power = system->getPower(); 
    
    // alignment values for screen content
    int leftCol = 10;
    int rightCol = 80; 
    int y = 26;
    int step = 14; 

    gfx->setTextSize(1);

    // extendet drawing operations for first screen draw
    // that static text dosnt flicker every update
    if (drawFirstTime == true) {
        drawFirstTime = false;

        //clear whole screen
        gfx->fillRect(0, 14, 160, 114, 0x0000);

        

        // --- SOFTWARE ---

        gfx->setTextColor(0xFFFF);
        gfx->drawString(FIRMWARE_VERSION, rightCol, y);
        y += step;

        // --- HARDWARE ---
        y += 4; 
        gfx->setTextColor(0x7BEF);
        gfx->drawString("CPU FREQ:", leftCol, y);
        gfx->setTextColor(0x07E0); // Green
        gfx->drawString(String(ESP.getCpuFreqMHz()) + " MHz", rightCol, y);
        y += step;

        gfx->setTextColor(0x7BEF);
        gfx->drawString("FREE RAM:", leftCol, y);
        gfx->setTextColor(0xFFE0); // Yellow
        gfx->drawString(String(ESP.getFreeHeap() / 1024) + " KB", rightCol, y);
        y += step;

        // --- BATTERY & POWER ---
        y += 4; 
        
        float voltage = power->getBatteryVoltage();
        int percent = power->getBatteryPercentage();
        
        gfx->setTextColor(0x7BEF);
        gfx->drawString("BATT VOLT:", leftCol, y);
        gfx->setTextColor(0x07FF); // Cyan
        gfx->drawString(String(voltage, 2) + " V", rightCol, y); 
        y += step;

        gfx->setTextColor(0x7BEF);
        gfx->drawString("BATT %:", leftCol, y);
        
        uint16_t battColor = (percent > 20) ? 0x07E0 : 0xF800; // Green or Red
        gfx->setTextColor(battColor); 
        gfx->drawString(String(percent) + " %", rightCol, y);
        y += step;


    }

    gfx->fillRect(0, 14, 160, 114, 0x0000);
    disp->drawHeader("SYSTEM INFO");

    

    gfx->setTextSize(1);

    // --- SOFTWARE ---

    gfx->setTextColor(0xFFFF);
    gfx->drawString(FIRMWARE_VERSION, rightCol, y);
    y += step;

    // --- HARDWARE ---
    y += 4; 
    gfx->setTextColor(0x7BEF);
    gfx->drawString("CPU FREQ:", leftCol, y);
    gfx->setTextColor(0x07E0); // Green
    gfx->drawString(String(ESP.getCpuFreqMHz()) + " MHz", rightCol, y);
    y += step;

    gfx->setTextColor(0x7BEF);
    gfx->drawString("FREE RAM:", leftCol, y);
    gfx->setTextColor(0xFFE0); // Yellow
    gfx->drawString(String(ESP.getFreeHeap() / 1024) + " KB", rightCol, y);
    y += step;

    // --- BATTERY & POWER ---
    y += 4; 
    
    float voltage = power->getBatteryVoltage();
    int percent = power->getBatteryPercentage();
    
    gfx->setTextColor(0x7BEF);
    gfx->drawString("BATT VOLT:", leftCol, y);
    gfx->setTextColor(0x07FF); // Cyan
    gfx->drawString(String(voltage, 2) + " V", rightCol, y); 
    y += step;

    gfx->setTextColor(0x7BEF);
    gfx->drawString("BATT %:", leftCol, y);
    
    uint16_t battColor = (percent > 20) ? 0x07E0 : 0xF800; // Green or Red
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
    
    gfx->setTextColor(0x7BEF);
    gfx->drawString("MAC:", leftCol, y); 
    gfx->setTextColor(0xFFFF);
    gfx->drawString(macStr, rightCol - 30, y);
}