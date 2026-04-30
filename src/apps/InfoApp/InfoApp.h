/**
 * Project: Arcade Controller V0.2
 * File: InfoApp.h
 * Description: Displays system information (hardware, firmware, battery, MAC).
 */

#pragma once
#include "../../apps/App.h"
#include <TFT_eSPI.h>

class InfoApp : public App {
public:
    using App::App; 

    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    unsigned long lastUpdate = 0;
    bool drawFirstTime = true;
    
    // Constant for your firmware version
    const char* FIRMWARE_VERSION = "v0.2.0";

    void drawScreen();
};