#include "skinchanger.hpp"
#include "../sdk/entity.hpp"
#include "../sdk/classids.hpp"
#include "../menu/config.hpp"

namespace SkinChanger {
    void applyModel(Entity* weapon, Entity* viewModel, Item item) {
        weapon->nDT_BaseViewModel__m_nModelIndex() = Interfaces::modelInfo->getModelIndex(item.modelName);
        weapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() = item.index;
        viewModel->nDT_BaseViewModel__m_nModelIndex() = Interfaces::modelInfo->getModelIndex(item.modelName);
    }

    void applySkin(Entity* weapon, int paintkit, int stattrack, float wear) {
            if (paintkit == 0 && stattrack == 0 && wear == 0)
                return;
            
            PlayerInfo info;
            Interfaces::engine->getPlayerInfo(Interfaces::engine->getLocalPlayer(), info);
            weapon->nDT_ScriptCreatedItem__m_iAccountID() = info.xuid;
            weapon->nDT_ScriptCreatedItem__m_iItemIDHigh() = -1;
            weapon->nDT_BaseAttributableItem__m_nFallbackPaintKit() = paintkit;
            weapon->nDT_BaseAttributableItem__m_flFallbackWear() = wear;
            weapon->nDT_BaseAttributableItem__m_nFallbackStatTrak() = stattrack;
    }

    void run(FrameStage stage) {
        if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START || !EntityCache::localPlayer || !Interfaces::engine->isInGame() || EntityCache::localPlayer->nDT_BasePlayer__m_iHealth() == 0)
            return;
        
        // Knife changer
        Entity* curWeapon = Interfaces::entityList->getClientEntity((uintptr_t)EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
        Entity* viewModel = Interfaces::entityList->getClientEntity((uintptr_t)EntityCache::localPlayer->nDT_BasePlayer__m_hViewModel0() & 0xfff);
        
        if (!curWeapon || !viewModel || curWeapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() == ItemIndex::INVALID)
            return;

        if(curWeapon->clientClass()->m_ClassID == CKnife)
            if (CONFIGINT("knife model") != 0)
                applyModel(curWeapon, viewModel, nameToItemMap[knives[CONFIGINT("knife model")]]);

        // Skin changer
        std::array<unsigned long, 48> weapons = EntityCache::localPlayer->weapons();
        for (auto weaponHandle : weapons) {
            if (weaponHandle == 0xFFFFFFFF)
                break;
            
            Entity* weapon = Interfaces::entityList->getClientEntity(weaponHandle & 0xfff);

            if (!weapon || !viewModel || weapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() == ItemIndex::INVALID)
                continue;
        
            if (itemIndexToNameMap.find(weapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex()) != itemIndexToNameMap.end()) {
                const char* curWeaponName = itemIndexToNameMap[weapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex()];
                char buf[256] = "skin changer ";
                strcat(buf, curWeaponName);

                char paintkit[256];
                strcpy(paintkit, buf);
                strcat(paintkit, " paintkit");

                char wear[256];
                strcpy(wear, buf);
                strcat(wear, " wear");

                char stattrack[256];
                strcpy(stattrack, buf);
                strcat(stattrack, " stattrack");
                
                applySkin(weapon, CONFIGINT(paintkit), CONFIGINT(stattrack), CONFIGFLOAT(wear));
            }
        }
    }
}