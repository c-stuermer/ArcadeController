/**
 * Project: Arcade Controller V0.1
 * File: RechargeApp.cpp
 * Description: Logic for the charging screen.
 */

#include "RechargeApp.h"
#include "../../ArcadeController.h" 

void RechargeApp::start() {
    Serial.println("[App] Recharge Mode started");
}

void RechargeApp::update() {
    // Access power status
    int batt = system->getPower()->getBatteryPercentage();
    bool isUsb = system->getPower()->isUSBConnected();
    float voltage = system->getPower()->getBatteryVoltage();

    // Logic: What happens if USB is unplugged?
    if (!isUsb) {
         if (system->getPower()->isSwitchedOn()) {
             // Case: Switch is ON -> User wants to play, go to InputMonitor
             system->startApp(system->getInputMonitorApp()); 
         } else {
             // Case: Switch is OFF -> User unplugged cable, go to Deep Sleep
             system->getPower()->enterDeepSleep();
         }
         return;
    }
    
    // Draw Charging UI
    U8G2* gfx = system->getDisplay()->getGfx();
    
    // Example UI:
    gfx->setFont(u8g2_font_helvB14_tf);
    gfx->setCursor(10, 30);
    gfx->print("CHARGING");
    
    gfx->setFont(u8g2_font_6x10_tf);
    gfx->setCursor(10, 50);
    gfx->printf("%d%% (%.2fV)", batt, voltage);

    // Debug output
    // Serial.printf("Charging... %d%%\n", batt);
}

void RechargeApp::onInput(ControlEvent ev, EventType type) {
    // No input handling needed during charging
}