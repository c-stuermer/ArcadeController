/**
 * Project: Arcade Controller V1.3
 * File: MenuApp.cpp
 * Description: Implementation of the main menu logic and rendering.
 *              All drawing goes through the IDisplay primitives
 *              (clear / fillRect / drawText / drawHeader). No path to
 *              TFT_eSPI exists from this file.
 */

#include "MenuApp.h"
#include "../../config/Colors.h"

MenuApp::MenuApp(IDisplay*      display,
                 ISound*        sound,
                 ISetting*      settings,
                 IAppNavigator* appNavigator)
    : App(),
      display(display),
      sound(sound),
      settings(settings),
      appManager(appNavigator) {}

void MenuApp::start() {
    Serial.println("[APP] MenuApp starting...");
    categories.clear();

    // --- Category 1: APPLICATIONS ---
    // Auto-generated from all apps registered with a non-null label.
    // Adding a new app to the menu only requires a label in main.cpp.
    MenuCategory apps;
    apps.title = "APPLICATIONS";

    for (const auto& [id, entry] : appManager->getRegistry()) {
        if (!entry.label) continue;
        apps.items.push_back(MenuItem{entry.label, [this, id](){
            appManager->switchApp(id);
        }});
    }

    // --- Category 2: SETTINGS ---
    MenuCategory settingsCat;
    settingsCat.title = "SETTINGS";

    // 1. SOUND (ON/OFF)
    settingsCat.items.push_back(MenuItem{
        "Sound",
        [this](){
            uint8_t vol = settings->getVolume();
            // Simple toggle between 0 (OFF) and 100 (ON).
            // setVolume() persists AND applies in one call (observer).
            uint8_t next = (vol == 0) ? 100 : 0;
            settings->setVolume(next);
            menuDirty = true;
        },
        [this](){
            uint8_t vol = settings->getVolume();
            return (vol == 0) ? String("OFF") : String("ON");
        }
    });

    // 2. BRIGHTNESS (25% increments)
    settingsCat.items.push_back(MenuItem{
        "Brightness",
        [this](){
            uint8_t bright = settings->getBrightness();
            uint8_t next = (bright >= 100) ? 25 : bright + 25;
            // setBrightness() persists AND applies in one call (observer).
            settings->setBrightness(next);
            menuDirty = true;
        },
        [this](){
            uint8_t bright = settings->getBrightness();
            String res = String(bright) + "%";
            return res;
        }
    });

    // 3. BOOT MODE (NORMAL / STEALTH)
    settingsCat.items.push_back(MenuItem{
        "Boot Mode",
        [this](){
            uint8_t mode = settings->getBootMode();
            uint8_t next = (mode == 0) ? 1 : 0;
            settings->setBootMode(next);
            menuDirty = true;
        },
        [this](){
            uint8_t mode = settings->getBootMode();
            return (mode == 1) ? String("NORMAL") : String("STEALTH");
        }
    });

    // 4. SYSTEM INFO
    settingsCat.items.push_back(MenuItem{"Info", [this](){
        appManager->switchApp(AppId::Info);
    }});

    // Add categories to the main list
    categories.push_back(apps);
    categories.push_back(settingsCat);

    // Reset selection pointers
    currentItemIndex = 0;
    currentCatIndex  = 0;

    // Initial screen clear and force redraw
    display->clear();
    menuDirty = true;
}

void MenuApp::update() {
    // Continuously draw header (handles battery updates)
    display->drawHeader(categories[currentCatIndex].title);

    // Only redraw the menu body if a change occurred (prevents flickering)
    if (menuDirty) {
        drawMenu();
        menuDirty = false;
    }

    display->flush();
}

void MenuApp::drawMenu() {
    auto& items = categories[currentCatIndex].items;

    const int startY     = 25;
    const int lineHeight = 15;

    for (size_t i = 0; i < items.size(); i++) {
        int yPos = startY + (i * lineHeight);

        // 1. Get base name
        String menuText = String(items[i].name);

        // 2. Append dynamic value if a getValue function is provided
        if (items[i].getValue) {
            menuText += ": " + items[i].getValue();
        }

        // 3. Convert entirely to uppercase for retro look
        menuText.toUpperCase();

        // 4. Background bar (selection highlight) + text
        if (i == currentItemIndex) {
            display->fillRect(0, yPos - 4, 160, lineHeight, Colors::GREEN);
            display->drawText(5, yPos, menuText, Colors::BLACK, 1);
        } else {
            display->fillRect(0, yPos - 4, 160, lineHeight, Colors::BLACK);
            display->drawText(5, yPos, menuText, Colors::WHITE, 1);
        }
    }
}

void MenuApp::onInput(ControlEvent ev, EventType type) {
    if (type != EventType::PRESSED) return;

    sound->play(SoundEffect::CLICK);

    auto& currentItems = categories[currentCatIndex].items;
    bool needRedraw = false;

    switch (ev) {
        case ControlEvent::JOY_UP:
            currentItemIndex = (currentItemIndex > 0) ? currentItemIndex - 1 : currentItems.size() - 1;
            needRedraw = true;
            break;

        case ControlEvent::JOY_DOWN:
            currentItemIndex = (currentItemIndex < (int)currentItems.size() - 1) ? currentItemIndex + 1 : 0;
            needRedraw = true;
            break;

        case ControlEvent::JOY_LEFT:
            currentCatIndex = (currentCatIndex > 0) ? currentCatIndex - 1 : categories.size() - 1;
            currentItemIndex = 0;

            // Clear screen entirely on category change because the title bar needs to be redrawn
            display->clear();
            needRedraw = true;
            break;

        case ControlEvent::JOY_RIGHT:
            currentCatIndex = (currentCatIndex < (int)categories.size() - 1) ? currentCatIndex + 1 : 0;
            currentItemIndex = 0;

            display->clear();
            needRedraw = true;
            break;

        case ControlEvent::BTN_A:
        case ControlEvent::BTN_START:
            if (currentItemIndex < (int)currentItems.size()) {
                currentItems[currentItemIndex].action();
                // Flag a redraw just in case the action changed a displayed value
                needRedraw = true;
            }
            break;

        default: break;
    }

    if (needRedraw) {
        menuDirty = true;
    }
}
