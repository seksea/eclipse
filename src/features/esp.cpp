#include <mutex>
#include "esp.hpp"
#include "../menu/config.hpp"
#include "../sdk/entity.hpp"

void outlineText(ImDrawList* drawList, ImVec2 pos, ImColor color, const char* text) {
    drawList->AddText(ImVec2(pos.x - 1, pos.y), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
    drawList->AddText(ImVec2(pos.x + 1, pos.y), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
    drawList->AddText(ImVec2(pos.x, pos.y - 1), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
    drawList->AddText(ImVec2(pos.x, pos.y + 1), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
    drawList->AddText(pos, color, text);
}

namespace ESP {
    void draw(ImDrawList* drawList) {
		std::lock_guard<std::mutex> lock(EntityCache::entityCacheLock);
        for (auto e : EntityCache::entityCache) {
            if (e.boundingBox.y < 0 || isnan(e.boundingBox.y) ) 
                continue;
            switch (e.classID) {
                case 40: { // Player 
                    if (e.health > 0) {
                        if (CONFIGBOOL(e.teammate ? "team esp box" : "enemy esp box")) {
                            drawList->AddRect(ImVec2(e.boundingBox.x, e.boundingBox.y), ImVec2(e.boundingBox.z, e.boundingBox.w), ImColor(0, 0, 0, 160), 0.f, 0, 3.f);
                            drawList->AddRect(ImVec2(e.boundingBox.x, e.boundingBox.y), ImVec2(e.boundingBox.z, e.boundingBox.w), CONFIGCOL(e.teammate ? "team esp box color" : "enemy esp box color"));
                        }
                        if (CONFIGBOOL(e.teammate ? "team esp name" : "enemy esp name")) {
                            outlineText(drawList, ImVec2(e.boundingBox.x + ((e.boundingBox.z - e.boundingBox.x) - ImGui::CalcTextSize(e.info.name).x)/2, e.boundingBox.y - 13), CONFIGCOL(e.teammate ? "team esp name color" : "enemy esp name color"), e.info.name);
                        }
                        if (CONFIGBOOL(e.teammate ? "team esp healthbar" : "enemy esp healthbar")) {
                            ImVec2 healthbarBorderMin = ImVec2(e.boundingBox.x - 6, e.boundingBox.y - 1);
                            ImVec2 healthbarBorderMax = ImVec2(e.boundingBox.x - 2, e.boundingBox.w + 1);
                            ImVec2 healthbarMin = ImVec2(healthbarBorderMin.x + 1, healthbarBorderMax.y + 1 - (((float)e.health / 100.f) * (healthbarBorderMax.y - healthbarBorderMin.y)));
                            ImVec2 healthbarMax = ImVec2(healthbarBorderMax.x - 1, healthbarBorderMax.y - 1);
                            drawList->AddRectFilled(healthbarBorderMin, healthbarBorderMax, ImColor(0, 0, 0, 160));
                            drawList->AddRectFilled(healthbarMin, healthbarMax, CONFIGCOL(e.teammate ? "team esp healthbar color" : "enemy esp healthbar color"));

                            if (e.health < 100) {
                                char hpText[16];
                                snprintf(hpText, sizeof(hpText), "%i", e.health);
                                outlineText(drawList, ImVec2(healthbarMin.x + 1 - (ImGui::CalcTextSize(hpText).x / 2), healthbarMin.y - 13), ImColor(255, 255, 255, 200), hpText);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}