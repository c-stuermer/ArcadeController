/**
 * Project: Arcade Controller V1.1
 * File: MenuApp.cpp
 * Description: Implementation of the main menu logic and rendering.
 */

#include "MenuApp.h"
#include "../ISystem.h"
#include "../AppManager.h"
#include "../../hal/DisplayManager.h"
#include "../../hal/SoundManager.h"
#include "../../hal/SettingsManager.h"

void MenuApp::start() {
    Serial.println("[APP] MenuApp starting...");
    categories.clear();

    // --- Category 1: APPLICATIONS ---
    MenuCategory apps;
    apps.title = "APPLICATIONS";
    
    apps.items.push_back(MenuItem{"INPUT MONITOR", [this](){
        system->getAppManager()->startApp(AppId::InputMonitor);
    }});

    apps.items.push_back(MenuItem{"BLUETOOTH", [this](){
        system->getAppManager()->startApp(AppId::Bluetooth);
    }});

    apps.items.push_back(MenuItem{"SPACE INVADERS", [this](){
        system->getAppManager()->startApp(AppId::SpaceInvaders);
    }});
    
    // --- Category 2: SETTINGS ---
    MenuCategory settings; 
    settings.title = "SETTINGS";
    
    // 1. SOUND (ON/OFF)
    settings.items.push_back(MenuItem{
        "Sound",
        [this](){
            uint8_t vol = system->getSettings()->getVolume();
            // Simple toggle between 0 (OFF) and 100 (ON)
            uint8_t next = (vol == 0) ? 100 : 0;

            system->setVolume(next);
            menuDirty = true;
        },
        [this](){
            uint8_t vol = system->getSettings()->getVolume();
            return (vol == 0) ? String("OFF") : String("ON");
        }
    });

    // 2. BRIGHTNESS (25% increments)
    settings.items.push_back(MenuItem{
        "Brightness",
        [this](){
            uint8_t bright = system->getSettings()->getBrightness();
            uint8_t next = (bright >= 100) ? 25 : bright + 25;

            // setBrightness persists AND applies in one call.
            system->setBrightness(next);
            menuDirty = true;
        },
        [this](){
            uint8_t bright = system->getSettings()->getBrightness();
            // Assign to string first before returning to ensure proper memory handling
            String res = String(bright) + "%";
            return res;
        }
    });

    // 3. BOOT MODE (NORMAL / STEALTH)
    settings.items.push_back(MenuItem{
        "Boot Mode",
        [this](){
            uint8_t mode = system->getSettings()->getBootMode();
            uint8_t next = (mode == 0) ? 1 : 0;

            system->setBootMode(next);
            menuDirty = true;
        },
        [this](){
            uint8_t mode = system->getSettings()->getBootMode();
            return (mode == 1) ? String("NORMAL") : String("STEALTH");
        }
    });

    // 4. SYSTEM INFO
    settings.items.push_back(MenuItem{"Info", [this](){
        system->getAppManager()->startApp(AppId::Info);
    }});

    // Add categories to the main list
    categories.push_back(apps);
    categories.push_back(settings);
    
    // Reset selection pointers
    currentItemIndex = 0;
    currentCatIndex = 0;

    // Initial screen clear and force redraw
    system->getDisplay()->getGfx()->fillScreen(0x0000);
    menuDirty = true; 
}

void MenuApp::update() {
    // Continuously draw header (handles battery updates)
    system->getDisplay()->drawHeader(categories[currentCatIndex].title);

    // Only redraw the menu body if a change occurred (prevents flickering)
    if (menuDirty) {
        drawMenu();
        menuDirty = false; 
    }
}

void MenuApp::drawMenu() {
    auto* gfx = system->getDisplay()->getGfx();
    auto& items = categories[currentCatIndex].items;

    gfx->setTextSize(1); 
    
    int startY = 25; 
    int lineHeight = 15; 
    uint16_t greenColor = 0x07E0; 

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

        if (i == currentItemIndex) {
            // Selected item: Green background bar (14px high for text size 1), black text
            gfx->fillRect(0, yPos - 4, 160, lineHeight, greenColor); 
            gfx->setTextColor(0x0000); 
        } else {
            // Unselected item: Black background, white text
            gfx->fillRect(0, yPos - 4, 160, lineHeight, 0x0000); 
            gfx->setTextColor(0xFFFF); 
        }

        gfx->setCursor(5, yPos);
        
        // 4. Draw the constructed text line
        gfx->print(menuText);
    }
}

void MenuApp::onInput(ControlEvent ev, EventType type) {
    if (type != EventType::PRESSED) return;

    system->getSound()->play(SoundEffect::CLICK);

    auto& currentItems = categories[currentCatIndex].items;
    bool needRedraw = false;

    switch (ev) {
        case ControlEvent::JOY_UP:
            currentItemIndex = (currentItemIndex > 0) ? currentItemIndex - 1 : currentItems.size() - 1;
            needRedraw = true;
            break;
            
        case ControlEvent::JOY_DOWN:
            currentItemIndex = (currentItemIndex < currentItems.size() - 1) ? currentItemIndex + 1 : 0;
            needRedraw = true;
            break;
            
        case ControlEvent::JOY_LEFT:
            currentCatIndex = (currentCatIndex > 0) ? currentCatIndex - 1 : categories.size() - 1;
            currentItemIndex = 0; 
            
            // Clear screen entirely on category change because the title bar needs to be redrawn
            system->getDisplay()->getGfx()->fillScreen(0x0000);
            needRedraw = true;
            break;
            
        case ControlEvent::JOY_RIGHT:
            currentCatIndex = (currentCatIndex < categories.size() - 1) ? currentCatIndex + 1 : 0;
            currentItemIndex = 0;
            
            system->getDisplay()->getGfx()->fillScreen(0x0000);
            needRedraw = true;
            break;
            
        case ControlEvent::BTN_A:
        case ControlEvent::BTN_START:
            if (currentItemIndex < currentItems.size()) {
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