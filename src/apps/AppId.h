/**
 * Project: Arcade Controller V1.1
 * File: apps/AppId.h
 * Description: Stable identifiers for all available applications.
 *
 * This enum is the only thing the rest of the system needs to know about
 * the app catalogue. Concrete app classes (MenuApp, BluetoothApp, ...)
 * stay encapsulated inside AppManager.
 */

#pragma once
#include <stdint.h>

enum class AppId : uint8_t {
    Menu,
    Bluetooth,
    Info,
    InputMonitor,
    SpaceInvaders
};
