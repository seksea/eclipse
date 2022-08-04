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
        if (!Interfaces::engine->isInGame() || !EntityCache::localPlayer)
            return;

        for (auto e : EntityCache::entityCache) {
            if (e.boundingBox.y < 0 || isnan(e.boundingBox.y) ) 
                continue;
            switch (e.type) {
                case EntityCache::EntityType::PLAYER: { // Player 
                    if (e.health > 0 && (CONFIGBOOL(e.teammate ? "teammate visible only" : "enemy visible only") ? e.visible : true)) {
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
                case EntityCache::EntityType::DROPPEDWEAPON: { // Weapon
                    ImColor boxColor = CONFIGCOL("weapon box color");
                    ImColor nameColor = CONFIGCOL("weapon name color");
                    QAngle viewAngles;
                    Interfaces::engine->getViewAngles(viewAngles);
                    if (CONFIGBOOL("dynamic weapon transparency")) {
                        QAngle angleToEnt = calcAngle(EntityCache::localPlayer->eyepos(), e.origin) - viewAngles;
                        normalizeAngles(angleToEnt);
                        boxColor.Value.w = 1-(angleToEnt.Length() / 40);
                        boxColor.Value.w = std::clamp(boxColor.Value.w, 0.2f, 1.f);
                        nameColor.Value.w = boxColor.Value.w;
                    }

                    if (CONFIGBOOL("weapon box")) {
                        drawList->AddRect(ImVec2(e.boundingBox.x, e.boundingBox.y), ImVec2(e.boundingBox.z, e.boundingBox.w), ImColor(0.f, 0.f, 0.f, boxColor.Value.w/2), 0.f, 0, 3.f);
                        drawList->AddRect(ImVec2(e.boundingBox.x, e.boundingBox.y), ImVec2(e.boundingBox.z, e.boundingBox.w), boxColor);
                    }
                    if (CONFIGBOOL("weapon name")) {
                        outlineText(drawList, ImVec2(e.boundingBox.x + ((e.boundingBox.z - e.boundingBox.x) - ImGui::CalcTextSize(e.weaponName.data()).x)/2, e.boundingBox.y - 13), nameColor, e.weaponName.data());
                    }
                    break;
                }
            }
        }
    }
}