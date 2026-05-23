/**
 * Project: Arcade Controller V1.4
 * File: main.cpp
 * Description: Composition root. Constructs all subsystems and apps
 *              and wires them together.
 *
 *              Global static instances are constructed in declaration
 *              order — SystemManager first (so AppManager can receive
 *              its IInputReader), then AppManager, then the apps which
 *              receive non-owning interface pointers from both.
 */

#include <Arduino.h>

#include <SystemManager.h>
#include <apps/AppManager.h>
#include <apps/MenuApp/MenuApp.h>
#include <apps/InfoApp/InfoApp.h>
#include <apps/InputMonitorApp/InputMonitorApp.h>
#include <apps/BluetoothApp/BluetoothApp.h>
#include <apps/SpaceInvadersApp/SpaceInvadersApp.h>
#include <apps/SnakeApp/SnakeApp.h>
// FroggerApp — work in progress, not included in this release


// --- systemManager + AppManager ------------------------------------------------
static SystemManager systemManager;
static InputHandler  inputHandler(systemManager.getInputReader());
static AppManager    appManager(inputHandler);

// --- Apps (composition-root-owned, interface-injected) -------------------
static MenuApp menu(
    systemManager.getDisplay(),
    systemManager.getSound(),
    systemManager.getSettings(),
    &appManager);

static InfoApp info(
    systemManager.getDisplay(),
    systemManager.getPower(),
    &appManager);

static InputMonitorApp inputMonitor(
    systemManager.getDisplay(),
    appManager.getInput(),   // InputHandler* -> IInputH*
    &appManager);

static BluetoothApp bluetooth(
    systemManager.getDisplay(),
    systemManager.getPower(),
    systemManager.getSettings(),
    appManager.getInput(),
    &appManager);

static SpaceInvadersApp spaceInvaders(
    systemManager.getDisplay(),
    systemManager.getSound(),
    appManager.getInput(),
    &appManager);

static SnakeApp snake(
    systemManager.getDisplay(),
    systemManager.getSound(),
    appManager.getInput(),
    &appManager);

void setup() {
    systemManager.begin();

    appManager.registerApp(AppId::Menu,         &menu);
    appManager.registerApp(AppId::Info,         &info);
    appManager.registerApp(AppId::InputMonitor, &inputMonitor, "INPUT MONITOR");
    appManager.registerApp(AppId::Bluetooth,    &bluetooth,    "BLUETOOTH");
    appManager.registerApp(AppId::SpaceInvaders,&spaceInvaders,"SPACE INVADERS");
    appManager.registerApp(AppId::Snake,        &snake,        "SNAKE");
    appManager.setDefaultApp(AppId::Menu);

    appManager.begin();
}

void loop() {
    systemManager.update();
    appManager.update();
}