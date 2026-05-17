/**
 * Project: Arcade Controller V1.2
 * File: main.cpp
 * Description: Entry point for the firmware.
 */

#include <ArcadeController.h>

// Create the global controller instance
ArcadeController arcade;

void setup() {
    // Initialize the main controller and all underlying hardware/software layers
    arcade.begin();
}

void loop() {
    // The main loop delegates all continuous logic to the controller
    arcade.update();
}