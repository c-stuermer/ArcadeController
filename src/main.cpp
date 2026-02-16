/**
 * Project: Arcade Controller V0.1
 * File: main.cpp
 * Description: Entry point for the firmware.
 * Note: This version uses an I2C display which limits the loop time to ~38ms.
 */

#include <ArcadeController.h>

// Create the global controller instance
ArcadeController arcade;

void setup() {
    // Initialize the controller and hardware
    arcade.begin();
}

void loop() {
    // Main loop delegates all logic to the controller
    arcade.update();
}