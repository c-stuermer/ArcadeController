/**
 * Project: Arcade Controller V1.3
 * File: InfoApp.h
 * Description: Displays system information (hardware, firmware, battery, MAC).
 *              V1.3: interface-injected, no ISystem dependency.
 */

#pragma once
#include "../App.h"
#include "../AppId.h"
#include "../../hal/interfaces/IDisplay.h"
#include "../../hal/interfaces/IPower.h"

#include "../interfaces/IAppNavigator.h"

class InfoApp : public App {
public:
    InfoApp(IDisplay* display, IPower* power, IAppNavigator* appNavigator);

    void start() override;
    void update() override;
    void stop() override;
    void onInput(ControlEvent ev, EventType type) override;

private:
    // Injected subsystems (non-owning).
    IDisplay*   display;
    IPower*     power;
    IAppNavigator* appManager;

    unsigned long lastUpdate = 0;

    // Firmware version (single source of truth, displayed in InfoApp)
    const char* FIRMWARE_VERSION = "V1.4";

    void drawScreen();
};
