#include <mutex>
#include "glow.hpp"
#include "../menu/config.hpp"
#include "../sdk/entity.hpp"
#include "lua.hpp"

namespace Glow {

    void refreshGlowEntities() {
        // clear old custom entities
        for (auto& entities : customGlowEntities)
            Interfaces::glowManager->unregisterGlowObject(entities.second);
        customGlowEntities.clear();
        
        // fill custom entities
        const auto highestEntityIndex = Interfaces::entityList->getHighestEntityIndex();
        for (int i = Interfaces::engine->getMaxClients() + 1; i <= highestEntityIndex; ++i) {
            const auto entity = Interfaces::entityList->getClientEntity(i);
            if (!entity || entity->dormant())
                continue;

            switch (entity->clientClass()->m_ClassID) {
            case ClassId::CBaseCSGrenadeProjectile:
            case ClassId::CBreachChargeProjectile:
            case ClassId::CBumpMineProjectile:
            case ClassId::CDecoyProjectile:
            case ClassId::CMolotovProjectile:
            case ClassId::CSensorGrenadeProjectile:
            case ClassId::CSmokeGrenadeProjectile:
            case ClassId::CSnowballProjectile:
                if (!Interfaces::glowManager->hasGlowEffect(entity)) {
                    int index = Interfaces::glowManager->registerGlowObject(entity);
                    if (index != -1)
                        customGlowEntities.emplace_back(i, index);
                }
                break;
            }
        }
    }

    void draw() {
        if (!EntityCache::localPlayer)
            return;

        refreshGlowEntities();

        Lua::handleHook("doPostScreenEffects");

	    for (int i = 0; i < Interfaces::glowManager->glowObjectDefinitions.Count(); i++) {
		    GlowObjectDefinition& glowObject = Interfaces::glowManager->glowObjectDefinitions[i];
            if (glowObject.isUnused() || !glowObject.entity)
                continue;
            
            switch (glowObject.entity->clientClass()->m_ClassID) {
                case CCSPlayer: {
                    if (glowObject.entity->teammate() ? CONFIGBOOL("glow teammate") : CONFIGBOOL("glow enemy") && ((CONFIGBOOL(glowObject.entity->teammate() ? 
                    "teammate legit esp" : "enemy legit esp")) ? glowObject.entity->nDT_BaseEntity__m_bSpotted() : true)) {
                        glowObject.renderWhenOccluded = true;
                        ImColor glowCol = glowObject.entity->teammate() ? CONFIGCOL("glow teammate color") : CONFIGCOL("glow enemy color");
                        glowObject.glowAlpha = glowCol.Value.w;
                        glowObject.glowStyle = glowObject.entity->teammate() ? CONFIGINT("glow teammate style") :  CONFIGINT("glow enemy style");
                        glowObject.glowColor = {glowCol.Value.x, glowCol.Value.y, glowCol.Value.z};
                    }
                    break;
                }
                case ClassId::CBaseCSGrenadeProjectile:
                case ClassId::CBreachChargeProjectile:
                case ClassId::CBumpMineProjectile:
                case ClassId::CDecoyProjectile:
                case ClassId::CMolotovProjectile:
                case ClassId::CSensorGrenadeProjectile:
                case ClassId::CSmokeGrenadeProjectile:
                case ClassId::CSnowballProjectile: {
                    if (CONFIGBOOL("glow grenade")) {
                        glowObject.renderWhenOccluded = true;
                        ImColor glowCol = CONFIGCOL("glow grenade color");
                        glowObject.glowAlpha = glowCol.Value.w;
                        glowObject.glowStyle = CONFIGINT("glow grenade style");
                        glowObject.glowColor = {glowCol.Value.x, glowCol.Value.y, glowCol.Value.z};
                    }
                    break;
                }
            }
			if ((glowObject.entity->clientClass()->m_ClassID != ClassId::CBaseWeaponWorldModel && strstr(glowObject.entity->clientClass()->m_pNetworkName, "Weapon")) || glowObject.entity->clientClass()->m_ClassID == ClassId::CDEagle || glowObject.entity->clientClass()->m_ClassID == ClassId::CC4 || glowObject.entity->clientClass()->m_ClassID == ClassId::CAK47) { // if is weapon
				if (glowObject.entity->nDT_BaseCombatWeapon__m_hOwner() == -1) {
                    if (CONFIGBOOL("glow weapon")) {
                        glowObject.renderWhenOccluded = true;
                        ImColor glowCol = CONFIGCOL("glow weapon color");
                        glowObject.glowAlpha = glowCol.Value.w;
                        glowObject.glowStyle = CONFIGINT("glow weapon style");
                        glowObject.glowColor = {glowCol.Value.x, glowCol.Value.y, glowCol.Value.z};
                    }
                }
            }
        }
    }
}