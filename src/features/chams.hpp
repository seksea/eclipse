#pragma once
#include "../interfaces.hpp"
#include <map>
#include <string>
#include <vector>

namespace Chams {
    inline std::map<std::string_view, IMaterial*> materials;

    inline void chamsCombo(const char* label, std::string& var, ImColor& col) {
        ImGui::Text("%s", label);
        char buf[128] = "##";
        strcat(buf, label);
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 20);
        if (ImGui::BeginCombo(buf, var.c_str())) {
            for (auto material : Chams::materials) {
                const bool is_selected = strstr(var.c_str(), material.first.data());
                if (ImGui::Selectable(material.first.data(), is_selected))
                    var = material.first.data();

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        strcat(buf, " color");
        ImGui::ColorEdit4(buf, (float*)&col.Value, ImGuiColorEditFlags_NoInputs);
    }

    void createMaterials();
    void doChams(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld);
}