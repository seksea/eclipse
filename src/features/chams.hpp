#pragma once
#include "../interfaces.hpp"
#include "../sdk/keyvalues.hpp"
#include <map>
#include <string>
#include <vector>

namespace Chams {
    inline std::map<std::string_view, IMaterial*> materials;

    inline IMaterial* createMaterial(const char* materialName, const char* materialType, const char* material) {
        KeyValues* keyValues = new KeyValues(materialName);

        typedef void (*InitKeyValues)(KeyValues*, const char*,int,int);
        static InitKeyValues initKeyValues = (InitKeyValues)Memory::patternScan("/client_client.so", "81 27 00 00 00 FF 55 45 31 C0 48 89 E5 5D"); // xref: OldParticleSystem_Destroy
        initKeyValues(keyValues, materialType,0,0);

        typedef void (*LoadFromBuffer)(KeyValues*, const char*, const char*, void*, const char*, void*);
        static LoadFromBuffer loadFromBuffer = (LoadFromBuffer)Memory::patternScan("/client_client.so", "55 48 89 E5 41 57 41 56 41 55 41 54 49 89 D4 53 48 81 EC ? ? ? ? 48 85");
        loadFromBuffer(keyValues, materialName, material, nullptr, nullptr, nullptr);

        return Interfaces::materialSystem->createMaterial(materialName, keyValues);
    }

    inline void addMaterial(const char* materialName, const char* materialType, const char* material) {
        materials[materialName] = createMaterial(materialName, materialType, material);
    }
    
    inline void removeMaterial(const char* materialName) {
        if (materials.find(materialName) == materials.end())
            return;
        materials.erase(materialName);
    }

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