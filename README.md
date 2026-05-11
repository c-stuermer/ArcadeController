# DIY Arcade Controller Firmware (ESP32)

![Project Status](https://img.shields.io/badge/Status-Active-brightgreen)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A modular, C++ based firmware for a custom-built Arcade Controller. Powered by an ESP32 with an MCP23017 I/O expander.

> **V1.0 ARCHITECTURE OVERHAUL**
>
> This release decouples the App layer from the composition root via a narrow `ISystem` service interface. App lifecycle and ownership have moved from `ArcadeController` into a dedicated `AppManager`, the gamepad transport is now fully owned by the `BluetoothApp` (BLE-HID active, Switch HID profile planned), and a `SpaceInvadersApp` mini-game has joined the catalogue.
>
> **PROJECT STATUS**
>
> Originally developed for a Cyber-Physical Systems module, V1.0 marks the first major release with a stable, decoupled architecture. Active development continues.

## Features

* **Eager-Press & Low-Latency Input:** The button debouncing (`Button` class) uses an "eager-press" logic. A button press is registered immediately, while the release is delayed (debounced) to ensure signal stability.
* **Decoupled App Layer via Service Interface:** Apps depend only on the slim `ISystem` interface, not on the concrete `ArcadeController`. The `AppManager` owns all `App` instances, the `InputHandler`, and routes input to the active app. App switching is identifier-based via the `AppId` enum — no app needs to know about another.
* **Modular App Catalogue:** `MenuApp` (settings & navigation), `BluetoothApp` (gamepad UI), `InfoApp` (system info), `InputMonitorApp` (input visualizer & latency display), `SpaceInvadersApp` (retro mini-game).
* **Multi-Mode Gamepad Transport:** `BluetoothApp` owns the gamepad adapter (BLE-HID via NimBLE active today; Switch HID profile planned) behind the `IGamepadOutput` interface. The active mode is switchable at runtime and persisted in flash, so the last-used profile is restored whenever the user opens `BluetoothApp`.
* **Emergency Exit Combo:** Hold `SELECT + L2 + R2` for 2 seconds inside `InputMonitorApp` or `SpaceInvadersApp` to return to the main menu — with a visual progress bar to confirm the combo.
* **Hardware Abstraction Layer (HAL):** Clean boundary between hardware logic (`PowerManager`, `DisplayManager`, `InputHandler`, `SoundManager`, `SettingsManager`) and the application layer. Subsystems are owned solely by the composition root.
* **Smart Power Management**
    * Deep sleep with wake-up via the physical power switch.
    * System reset wired to the ESP32 hardware EN-pin (no software involvement).
    * Display states are frozen via `gpio_hold_en()` during sleep to prevent backlight ghost-glow and battery drain.
* **Persistent System Settings:** Brightness, volume, boot mode (Normal/Stealth), and gamepad mode (BLE-HID/Switch) are stored in flash via the ESP32 Preferences library and re-applied on boot.
* **Pull-Based Battery Sync:** Display and gamepad consumers each pull the current battery percentage from `PowerManager` on their own cadence — no central push, no coupling between subsystems.

## 🛠 Hardware Setup

This firmware is designed for the following hardware configuration:

* **MCU:** ESP32 (Tested on DFRobot FireBeetle 2 ESP32-E)
* **Expander:** MCP23017 (I2C Address `0x20`)
* **Display:** SPI TFT Display, ST7735 driver, 128 × 160 px (e.g. 1.8" ST7735 GreenTab2 module)
* **Sound:** Piezo/Speaker connected to a PWM pin (Hardware LEDC Synthesis)
* **Controls:**
    * 1x Joystick (4 Microswitches)
    * 8x Arcade Action Buttons (A, B, X, Y, L1, R1, L2, R2)
    * 2x System Buttons (Select, Start)
    * 1x Power Switch (toggle, wakes from deep sleep)
    * 1x Reset Button (wired to ESP32 EN-pin)

### Pinout Configuration

*See `src/config/Config.h` for exact pin mappings.*

## 🏗 Architecture

* **Composition Root:** `ArcadeController` owns the four HAL singletons and the `AppManager`. It implements `ISystem` and is the only class that knows the full picture.
* **Service Interface (`ISystem`):** The contract apps see — accessors for the HAL managers, the `AppManager`, the `InputHandler`, and cross-cutting setters that persist *and* apply (e.g. `setBrightness()`).
* **App Layer:** `AppManager` owns `InputHandler` and every `App` instance. Apps inherit from the abstract `App` base class and receive an `ISystem*` on construction. App switching goes through `AppManager::startApp(AppId)`.
* **Transport:** `BluetoothApp` owns the `BleGamepadAdapter` (active) and reserves a slot for a `Switch2GamepadAdapter` (planned, see Roadmap), both behind the `IGamepadOutput` interface so the active mode can be switched at runtime.

## 📂 Project Structure

```text
/src
 ├── main.cpp                       # Entry point
 ├── ArcadeController.*             # Composition root, implements ISystem
 ├── /apps
 │    ├── ISystem.h                 # Service interface for the App layer
 │    ├── App.h                     # Abstract base class for all apps
 │    ├── AppId.h                   # Stable identifiers for app switching
 │    ├── AppManager.*              # Owns InputHandler + all App instances
 │    ├── /MenuApp                  # Main menu & settings
 │    ├── /BluetoothApp             # Gamepad UI + transport ownership
 │    ├── /InfoApp                  # System & hardware info screen
 │    ├── /InputMonitorApp          # Input visualizer & latency display
 │    └── /SpaceInvadersApp         # Retro mini-game
 ├── /config
 │    └── Config.h                  # Pin mappings & ControlEvent enum
 ├── /driver
 │    └── Button.*                  # Eager-press debounce logic
 ├── /hal
 │    ├── DisplayManager.*          # TFT_eSPI wrapper & UI primitives
 │    ├── InputHandler.*            # GPIO + MCP23017 polling
 │    ├── PowerManager.*            # ADC battery measurement & deep sleep
 │    ├── SettingsManager.*         # Persistent flash storage (Preferences)
 │    └── SoundManager.*            # Non-blocking PWM sound synthesis
 └── /transport
      ├── IGamepadOutput.h          # Gamepad transport interface
      └── BLEGamepadAdapter.*       # NimBLE BLE-HID implementation
```

## ⚠ Known Issues

* **BLE-HID battery level reporting:** The controller calls `setBatteryLevel()` on every battery-percentage change, but some hosts do not display the value correctly. The root cause is suspected in the HID battery-service initialization order rather than the firmware logic; investigation is open and will be tracked for a follow-up release.

## 🗺 Roadmap

### Planned for V1.1: Auto-Generated App Registry

Currently, adding a new app requires touching three places:

1. The `AppId` enum in `apps/AppId.h`
2. Member declaration, constructor init, and `resolveApp()` switch in `AppManager`
3. The `MenuApp` to add the menu entry

The plan for V1.1 is to consolidate this into a **single source of truth** — one central
`apps/AppList.h` file containing every app exactly once, with the `AppId` enum,
`AppManager` members, the `resolveApp` switch, and the `MenuApp` entries all generated
from that single list.

**Likely approach:** X-macro pattern. The list is defined as a higher-order macro, then
expanded multiple times with different per-row definitions to produce enum values, member
declarations, switch cases, and menu items — all from the same authoritative list.

**Goal in numbers:** Adding a new app drops from 3 manual edits to 1 list entry.