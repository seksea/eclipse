#include "skinchanger.hpp"
#include "../sdk/entity.hpp"
#include "../sdk/classids.hpp"
#include "../menu/config.hpp"

namespace SkinChanger {
    void run(FrameStage stage) {
        if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && EntityCache::localPlayer && Interfaces::engine->isInGame() && EntityCache::localPlayer->nDT_BasePlayer__m_iHealth() > 0) {
            Entity* curWeapon = Interfaces::entityList->getClientEntity((uintptr_t)EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
            if (curWeapon && curWeapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() != ItemIndex::INVALID) {
                if (curWeapon->clientClass()->m_ClassID == CKnife) {
                    if (CONFIGINT("knife model")) {
                        Item item = nameToItemMap[knives[CONFIGINT("knife model")]];
                        curWeapon->nDT_BaseViewModel__m_nModelIndex() = Interfaces::modelInfo->getModelIndex(item.modelName);
                        curWeapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() = item.index;
                        Entity* viewModel = Interfaces::entityList->getClientEntity((uintptr_t)EntityCache::localPlayer->nDT_BasePlayer__m_hViewModel0() & 0xfff);
                        if (viewModel) {
                            viewModel->nDT_BaseViewModel__m_nModelIndex() = Interfaces::modelInfo->getModelIndex(item.modelName);
                        }
                    }
                }
            }
        }
    }
}