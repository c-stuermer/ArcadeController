# DIY Arcade Controller Firmware (ESP32)

![Project Status](https://img.shields.io/badge/Status-Legacy%20Snapshot-yellow)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A modular, C++ based firmware for a custom-built DIY Arcade Controller using an ESP32 (FireBeetle), MCP23017 I/O expander, and an OLED display.

> **⚠️ V0.1 ARCHIVED SNAPSHOT**
> 
> This version (V0.1) serves as a "save point". It utilizes an **I2C OLED Display (SH1106)**. Due to the I2C bus bandwidth and driver overhead, the main loop is limited to approx. **~26Hz (38ms latency)**.
>
> **Future Plans:** The next version (V0.2) will migrate to an SPI-based display to significantly reduce input lag for competitive gaming.

## 🎮 Features

* **Modular App System:** The firmware is built around an `AppManager` that allows switching between different "Apps" (e.g., Input Monitor, Charging Screen, Menu).
* **Bluetooth LE Gamepad:** Acts as a standard HID Gamepad compatible with Windows, macOS, Android, and iOS.
* **Hardware Abstraction Layer (HAL):** Clean separation between hardware logic (Power, Display, Input, Sound) and application logic.
* **Smart Power Management:**
    * Deep Sleep support.
    * Battery monitoring (Voltage & Percentage).
    * Auto-wake on physical switch.
    * Dedicated charging mode when USB is connected while switched off.
* **Input Handling:**
    * Direct GPIO inputs (Joystick).
    * I2C Port Expander (MCP23017) for Arcade Buttons.
    * Software Debouncing.

## 🛠 Hardware Setup

This firmware is designed for the following hardware configuration:

* **MCU:** ESP32 (Tested on DFRobot FireBeetle 2 ESP32-E)
* **Expander:** MCP23017 (I2C Address `0x20`)
* **Display:** 1.3" OLED SH1106 (I2C Address `0x3C`)
* **Sound:** Piezo/Speaker on DAC Pin via Amplifier
* **Controls:** * 1x Joystick (4 Switches)
    * 8x Arcade Action Buttons (A, B, X, Y, L1, R1, L2, R2)
    * 2x System Buttons (Select, Start)

### Pinout Configuration

*See `src/config/Config.h` for detailed mapping.*

## 📂 Project Structure

```text
/src
 ├── main.cpp              # Entry point
 ├── ArcadeController.* # Main Coordinator (Facade Pattern)
 ├── /apps                 # Application Logic
 │    ├── AppManager.* # Handles App switching
 │    ├── InputMonitorApp  # Visualizes inputs on OLED
 │    └── RechargeApp      # Handles charging state
 ├── /config               # Pin definitions & Constants
 ├── /driver               # Logic Drivers (Button Debouncing)
 ├── /hal                  # Hardware Abstraction Layer
 │    ├── DisplayManager   # U8g2 Wrapper
 │    ├── InputHandler     # GPIO & MCP23017 Polling
 │    ├── PowerManager     # Battery & Sleep logic
 │    └── SoundManager     # Non-blocking sound generation
 └── /transport            # Bluetooth Gamepad Implementation