/**
 * Project: Arcade Controller V0.1
 * File: RechargeApp.h
 * Description: Displays charging status and handles power transitions.
 */

#pragma once
#include "../App.h"

// Forward Declaration
class ArcadeController; 

class RechargeApp : public App {
public:
    using App::App; 

    void start() override;
    void update() override;
    void onInput(ControlEvent ev, EventType type) override;
};