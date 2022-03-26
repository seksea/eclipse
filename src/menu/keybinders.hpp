#pragma once
#include <SDL2/SDL_keyboard.h>
#include "keybinders.hpp"

enum class KeyBindTypes {
    ALWAYS, HOLD, TOGGLE, NONE
};

struct KeyBindConfigItem {
    int key;
    KeyBindTypes type;
    bool buttonToggled = false;
    bool toggled = false;
    KeyBindConfigItem(int k, KeyBindTypes t) {
        key = k;
        type = t;
    }
};

void drawKeyBinder(const char* configItemName);

bool isKeyBinderPressed(KeyBindConfigItem* item);