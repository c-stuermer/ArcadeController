# DIY Arcade Controller Firmware (ESP32)

![Project Status](https://img.shields.io/badge/Status-Active-brightgreen)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A modular, C++ based firmware for a custom-built Arcade Controller. Powered by an ESP32 with an MCP23017 I/O expander.

> **V1.2 — MAJOR REFACTORING**
>
> Project-wide cleanup and readability pass focused on the input signal path. The BLE transport, `InputHandler`, and `BluetoothApp` have been brought onto a consistent architecture: single mapping table as source of truth, explicit state machines, narrow public interfaces. Magic numbers and hex color literals were replaced project-wide with named constants (central `Colors` palette, display geometry, battery thresholds), and HAL member naming is unified. Functionally new: real edge-to-display latency measurement in `InputMonitorApp` and a targeted post-release hold-off on the joystick microswitches that suppresses pre-snap contact chatter on slow releases. Also includes dead-code removal across the public APIs and a rendering bug fix in `InfoApp`.
>
> **V1.1 — INPUT HANDLER REFINEMENT**
>
> `InputHandler` now publishes a debounced-state bitmap via `getDebouncedStates()`. Poll-style consumers such as `InputMonitorApp` read the entire input snapshot in a single register-level call instead of N individual `isPressed()` lookups. Bit positions mirror the `ControlEvent` enum, so the layout is self-documenting and shared across the project.
>
> **V1.0 — ARCHITECTURE OVERHAUL**
>
> This release decouples the App layer from the composition root via a narrow `ISystem` service interface. App lifecycle and ownership have moved from `ArcadeController` into a dedicated `AppManager`, the gamepad transport is now fully owned by the `BluetoothApp` (BLE-HID active, Switch HID profile slot reserved), and a `SpaceInvadersApp` mini-game has joined the catalogue.
>
> **PROJECT STATUS**
>
> Originally developed for a Cyber-Physical Systems module, V1.0 marks the first major release with a stable, decoupled architecture. Active development continues.

## Features

* **Eager-Press & Low-Latency Input:** The button debouncing (`Button` class) uses an "eager-press" logic. A button press is registered immediately, while the release is delayed (debounced) to ensure signal stability.
* **Self-Contained Button Entity:** Each `Button` owns its hardware identity (`PinType` + pin) and logical identity (`ControlEvent`) alongside its debounce state. `InputHandler` iterates a flat `std::vector<Button>` instead of a wrapper struct hierarchy.
* **Decoupled App Layer via Service Interface:** Apps depend only on the slim `ISystem` interface, not on the concrete `ArcadeController`. The `AppManager` owns all `App` instances, the `InputHandler`, and routes input to the active app. App switching is identifier-based via the `AppId` enum — no app needs to know about another.
* **Three-Pattern Input API:** `InputHandler` exposes the debounced input state in three complementary shapes — event callbacks for reactive consumers (`onEvent`), per-event polling for selective checks (`isPressed`, `getDuration`), and a compact 16-bit snapshot bitmap for bulk reads (`getDebouncedStates`). Each app picks the access pattern that fits its use case; debouncing lives exclusively in the HAL.
* **Modular App Catalogue:** `MenuApp` (settings & navigation), `BluetoothApp` (gamepad UI), `InfoApp` (system info), `InputMonitorApp` (input visualizer & latency display), `SpaceInvadersApp` (retro mini-game).
* **Multi-Mode Gamepad Transport:** `BluetoothApp` owns the gamepad adapter behind the `IGamepadOutput` interface. BLE-HID via NimBLE is the active transport today; the architecture supports multiple modes — switchable at runtime and persisted in flash, so the last-used profile is restored whenever the user opens `BluetoothApp`.
* **Single HID Mapping Table:** `BleGamepadAdapter` translates every `ControlEvent` to its HID button index through one static `HID_BUTTONS` table consulted by both `press()` and `release()`. Adding a new arcade button is a single row.
* **Centralized Color Palette:** All UI surfaces (HAL + apps) draw against named constants in `config/Colors.h` instead of inlined RGB565 literals. Display geometry (screen size, header, progress bar, battery indicator) is encoded as named constants on `DisplayManager`.
* **Emergency Exit Combo:** Hold `SELECT + L2 + R2` for 2 seconds inside `InputMonitorApp` or `SpaceInvadersApp` to return to the main menu — with a visual progress bar to confirm the combo.
* **Hardware Abstraction Layer (HAL):** Clean boundary between hardware logic (`PowerManager`, `DisplayManager`, `InputHandler`, `SoundManager`, `SettingsManager`) and the application layer. Subsystems are owned solely by the composition root.
* **Smart Power Management**
    * Deep sleep with wake-up via the physical power switch.
    * System reset wired to the ESP32 hardware EN-pin (no software involvement).
    * Display states are frozen via `gpio_hold_en()` during sleep to prevent backlight ghost-glow and battery drain.
* **Persistent System Settings:** Brightness, volume, boot mode (Normal/Stealth), and gamepad mode are stored in flash via the ESP32 Preferences library and re-applied on boot.
* **Pull-Based Battery Sync:** Display and gamepad consumers each pull the current battery percentage from `PowerManager` on their own cadence — no central push, no coupling between subsystems. Battery thresholds (full / empty / divider ratio) live as named constants on `PowerManager`, so the calibration is in one place.

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
* **Signal Path (Input → Host):** GPIO/MCP23017 → `Button` (eager-press debounce) → `InputHandler` (polls, accumulates bitmap, dispatches events) → `AppManager` (routes to active app) → `BluetoothApp` (passthrough state) → `BleGamepadAdapter` (`HID_BUTTONS` table + 8-way hat) → NimBLE → host.
* **Transport:** `BluetoothApp` owns the `BleGamepadAdapter` and reserves a slot for a `Switch2GamepadAdapter`, both behind the `IGamepadOutput` interface so the active mode can be switched at runtime.

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
 │    ├── Colors.h                  # Central RGB565 palette (named constants)
 │    └── Config.h                  # Pin mappings & ControlEvent enum
 ├── /driver
 │    └── Button.*                  # Eager-press debounce + flat identity
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
