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

inline bool isKeyBinderPressed(KeyBindConfigItem* item) {
    switch (item->type) {
        case KeyBindTypes::ALWAYS: {
            return true;
        }
        case KeyBindTypes::HOLD: {
            return item->key < 0 ? item->key == -1 ? ImGui::IsMouseDown(0) : 
                                    item->key == -2 ? ImGui::IsMouseDown(1) : 
                                    item->key == -3 ? ImGui::IsMouseDown(2) : 
                                    item->key == -4 ? ImGui::IsMouseDown(3) : 
                                    item->key == -5 ? ImGui::IsMouseDown(4) : false : 
                                    ImGui::IsKeyDown(item->key);
        }
        case KeyBindTypes::TOGGLE: {
            if (item->key < 0 ? item->key == -1 ? ImGui::IsMouseClicked(0) :
                                    item->key == -2 ? ImGui::IsMouseClicked(1) : 
                                    item->key == -3 ? ImGui::IsMouseClicked(2) : 
                                    item->key == -4 ? ImGui::IsMouseClicked(3) : 
                                    item->key == -5 ? ImGui::IsMouseClicked(4) : false :
                                    ImGui::IsKeyPressed(item->key))
                item->toggled = !item->toggled;

            return item->toggled;
        }
        case KeyBindTypes::NONE: {
            return false;
        }
    }
}