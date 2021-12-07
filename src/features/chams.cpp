#include "chams.hpp"
#include "../hooks.hpp"
#include "../menu/config.hpp"
#include "../sdk/entity.hpp"

#define ENEMYTEAMCONFIGSTR(var) (ent->teammate() ? CONFIGSTR("team " var) : CONFIGSTR("enemy " var))
#define ENEMYTEAMCONFIGCOL(var) (ent->teammate() ? CONFIGCOL("team " var) : CONFIGCOL("enemy " var))

namespace Chams {
    void chamEntity(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld, ImColor color, const std::string& materialName, bool ignoreZ = false) {
        if (materials.find(materialName) == materials.end() || materialName.length() == 0) {
            Hooks::DrawModelExecute::original(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
            return;
        }
        IMaterial* mat = materials[materialName];
        
        mat->alphaModulate(color.Value.w);
        mat->colorModulate(color.Value.x, color.Value.y, color.Value.z);
        mat->setMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
        bool found;
        IMaterialVar* var = mat->findVar("$envmaptint", &found);
        if (found) {
            var->setVecValue(color.Value.x, color.Value.y, color.Value.z);
        }
        Interfaces::modelRender->forcedMaterialOverride(mat);
        Hooks::DrawModelExecute::original(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
        Interfaces::modelRender->forcedMaterialOverride(nullptr);
    }

    void createMaterials() {
        materials[""] = nullptr;
        materials["normal"] = Interfaces::materialSystem->findMaterial("debug/debugambientcube", nullptr);
        materials["flat"] = Interfaces::materialSystem->findMaterial("debug/debugdrawflat", nullptr);
        materials["plastic"] = Interfaces::materialSystem->findMaterial("models/inventory_items/trophy_majors/gloss", nullptr);
    }

    void doChams(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld) {
        if (Interfaces::studioRender->overrideType == OverrideType::DepthWrite || Interfaces::studioRender->overrideType == OverrideType::SsaoDepthWrite) {
            Hooks::DrawModelExecute::original(thisptr, ctx, state, pInfo, pCustomBoneToWorld); // Draw Shadows
            return;
        }

        Entity* ent = Interfaces::entityList->getClientEntity(pInfo.entity_index);
        if (!ent) {
            Hooks::DrawModelExecute::original(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
            return;
        }

	    const char* modelName = Interfaces::modelInfo->getModelName(pInfo.pModel);
	    if (strstr(modelName, "models/player") && !strstr(modelName, "shadow")) {
            if (ENEMYTEAMCONFIGSTR("ignorez chams material").length() > 0)
                chamEntity(thisptr, ctx, state, pInfo, pCustomBoneToWorld, ENEMYTEAMCONFIGCOL("ignorez chams color"), ENEMYTEAMCONFIGSTR("ignorez chams material"), true);

            chamEntity(thisptr, ctx, state, pInfo, pCustomBoneToWorld, ENEMYTEAMCONFIGCOL("chams color"), ENEMYTEAMCONFIGSTR("chams material"));
            
            if (ENEMYTEAMCONFIGSTR("overlay chams material").length() > 0)
                chamEntity(thisptr, ctx, state, pInfo, pCustomBoneToWorld, ENEMYTEAMCONFIGCOL("overlay chams color"), ENEMYTEAMCONFIGSTR("overlay chams material"));
        }
        else {
            Hooks::DrawModelExecute::original(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
        }
    }
}