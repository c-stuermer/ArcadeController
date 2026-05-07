# DIY Arcade Controller Firmware (ESP32)

![Project Status](https://img.shields.io/badge/Status-Active-brightgreen)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A modular, C++ based firmware for a custom-built Arcade Controller. Powered by an ESP32, an MCP23017 I/O expander.

> **V0.2 PERFORMANCE UPDATE**
> 
> This version replaces the legacy I2C OLED with an SPI TFT display via **TFT_eSPI**. This eliminates the I2C bus bottleneck, drastically increasing the loop frequency and minimizing input latency.
>
> **PROJECT STATUS**
> 
> This repository contains the final, feature-complete release developed for a Cyber-Physical Systems module. While the core objectives have been successfully met and finalized, this project is still under development.

## Features

* **Eager-Press & Low-Latency Input:** The button debouncing (`Button` class) utilizes an "eager-press" logic. A button press is registered immediately with 0 ms latency, while the release is delayed (debounced) to ensure signal stability.
* **Modular App System:** A dedicated app manager (`AppManager`) switches between different system applications (`MenuApp`, `InfoApp`, `InputMonitorApp`, `BluetoothApp`).
* **Bluetooth LE Gamepad (NimBLE):** Acts as standard HID Gamepad. Features a local UI overlay and an "Emergency Exit" combo (Hold SELECT + L2 + R2 for 2 seconds) to safely disconnect the Bluetooth passthrough at any time.
* **Hardware Abstraction Layer (HAL):** Clean separation between raw hardware logic (Power, Display, Input, Sound) and the application layer.
* **Smart Power Management**
    * Deep sleep support with wake-up via the physical power switch.
    * Hardware states (like the display backlight) are frozen during sleep using `gpio_hold_en()` to prevent battery drain.
* **Persistent System Settings:** Brightness, volume, and boot mode (Normal/Stealth) are safely stored in flash memory using the ESP32 Preferences library.

> **Known Architectural Trade-offs**
> 
> **Separation of Concerns:** In the following version, a clearer boundary between the ArcadeController and the AppManager will be established. Currently, their responsibilities are somewhat blended, as the ArcadeController still handles the instantiation and ownership of specific App classes. Future refactoring aims to decouple these roles, delegating full lifecycle management to the AppManager while the ArcadeController focuses solely on hardware coordination.

## 🛠 Hardware Setup

This firmware is designed for the following hardware configuration:

* **MCU:** ESP32 (Tested on DFRobot FireBeetle 2 ESP32-E)
* **Expander:** MCP23017 (I2C Address `0x20`)
* **Display:** SPI TFT Display (TFT_eSPI compatible, e.g., ILI9341 / ST7789)
* **Sound:** Piezo/Speaker connected to a PWM pin (Hardware LEDC Synthesis)
* **Controls:** 
    * 1x Joystick (4 Microswitches)
    * 8x Arcade Action Buttons (A, B, X, Y, L1, R1, L2, R2)
    * 2x System Buttons (Select, Start)

### Pinout Configuration

*See `src/config/Config.h` for exact pin mappings.*

## 📂 Project Structure

```text
/src
 ├── main.cpp                 # Entry Point
 ├── ArcadeController.*       # Main Coordinator 
 ├── /apps                    # Application Logic
 │    ├── AppManager.*        # App Switching
 │    ├── MenuApp.*           # Main Menu & Settings
 │    ├── BluetoothApp.*      # BLE Passthrough & Pairing UI
 │    ├── InfoApp.*           # System & Hardware Info
 │    └── InputMonitorApp.*   # Input Visualizer 
 ├── /config                  # Pin definitions & Constants
 ├── /driver                  # Driver Logic (e.g., Button)
 ├── /hal                     # Hardware Abstraction Layer
 │    ├── DisplayManager.*    # TFT_eSPI Wrapper & UI Elements
 │    ├── InputHandler.*      # High-Speed GPIO & MCP23017 Polling
 │    ├── PowerManager.*      # ADC Battery Measurement & Deep Sleep
 │    ├── SettingsManager.*   # Persistent Flash Storage
 │    └── SoundManager.*      # Non-blocking PWM Sound Synthesis
 └── /transport               # Protocols
      ├── IGamepadOutput.h    # Interface for Gamepad usage
      └── BLEGamepadAdapter.* # NimBLE Gamepad Implementation
```