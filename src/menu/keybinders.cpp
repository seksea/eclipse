#include "config.hpp"
#include "imgui/imgui.h"

void drawKeyBinder(const char* configItemName) {
    KeyBindConfigItem* item = &CONFIGBIND(configItemName);
    char popupName[64] = "popup ";
    strcat(popupName, configItemName);

    char buf[32];
    if (item->buttonToggled) {
        strcpy(buf, "...");
    }
    else {
        strcpy(buf, item->key < 0 ? item->key == -1 ? "mouse1" :
                                    item->key == -2 ? "mouse2" : 
                                    item->key == -3 ? "mouse3" : 
                                    item->key == -4 ? "mouse4" : 
                                    item->key == -5 ? "mouse5" : "" :
                                    SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)item->key)));
        if (!strlen(buf)) {
            strcpy(buf, "n/a");
        }
    }

    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(buf).x) - 2);
    
    if (ImGui::Button(buf))
        item->buttonToggled = true;
    
    static KeyBindConfigItem* popupItem = item;

    if (ImGui::IsItemClicked(1)) {
        ImGui::OpenPopup(popupName);
        popupItem = item;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(50, 0));
    if (ImGui::BeginPopup(popupName)) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.14f, 0.14f, 0.15f, 1.00f));
        const char* arr[] = {"always", "hold", "toggle", "none"};
        ImGui::ListBox("##keybind type", (int*)&popupItem->type, arr, IM_ARRAYSIZE(arr));
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    if (item->buttonToggled) {
        if (item->type == KeyBindTypes::ALWAYS) {
            item->type = KeyBindTypes::HOLD;
        }
        for (auto i = 0; i < SDL_NUM_SCANCODES; i++) {
            if (ImGui::IsKeyPressed(i)) {
                item->key = i;
                item->buttonToggled = false;
                break;
            }
        }
        if (ImGui::IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
            item->key = 0;
            item->buttonToggled = false;
            item->type = KeyBindTypes::ALWAYS;
        }
        else if (ImGui::IsMouseDown(0)) {
            item->key = -1;
            item->buttonToggled = false;
        }
        else if (ImGui::IsMouseDown(1)) {
            item->key = -2;
            item->buttonToggled = false;
        }
        else if (ImGui::IsMouseDown(2)) {
            item->key = -3;
            item->buttonToggled = false;
        }
        else if (ImGui::IsMouseDown(3)) {
            item->key = -4;
            item->buttonToggled = false;
        }
        else if (ImGui::IsMouseDown(4)) {
            item->key = -5;
            item->buttonToggled = false;
        }
    }
}