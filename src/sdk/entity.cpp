#include "entity.hpp"
#include "classids.hpp"

bool Entity::visCheck() {
    if (!EntityCache::localPlayer)
        return false;

    matrix3x4_t boneMatrix[128];
    if (!this->setupBones(boneMatrix, 128, BONE_USED_BY_ANYTHING, Interfaces::globals->curtime))
        return false;
    
    TraceFilter filter;
    filter.pSkip = EntityCache::localPlayer;

    Trace traceToHead;
    Ray rayToHead;
    rayToHead.Init(EntityCache::localPlayer->eyepos(), Vector(boneMatrix[8][0][3], boneMatrix[8][1][3], boneMatrix[8][2][3]));
                                        // (MASK_SHOT)
    Interfaces::trace->traceRay(rayToHead, 1174421507, &filter, &traceToHead);

    Trace traceToUpperSpinal;
    Ray rayToUpperSpinal;
    rayToUpperSpinal.Init(EntityCache::localPlayer->eyepos(), Vector(boneMatrix[6][0][3], boneMatrix[6][1][3], boneMatrix[6][2][3]));
                                               // (MASK_SHOT)
    Interfaces::trace->traceRay(rayToUpperSpinal, 1174421507, &filter, &traceToUpperSpinal);

    return (traceToHead.m_pEntityHit == this) && (traceToUpperSpinal.m_pEntityHit == this) && 
            !Interfaces::lineGoesThroughSmoke(EntityCache::localPlayer->eyepos(), this->eyepos(), 1);
}

bool Entity::canShoot() {
    const float serverTime = TICKS_TO_TIME(this->nDT_LocalPlayerExclusive__m_nTickBase());
    Entity* weapon = (Entity*)Interfaces::entityList->getClientEntity(
         (uintptr_t)this->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xFFF);
    if (!weapon) return false;
    if (!weapon->nDT_BaseCombatWeapon__m_iClip1()) return false;
    if (this->nDT_BCCLocalPlayerExclusive__m_flNextAttack() > serverTime) return false;
    if (weapon->nDT_LocalActiveWeaponData__m_flNextPrimaryAttack() > serverTime) return false;
    if (weapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() == ItemIndex::WEAPON_REVOLVER &&
        weapon->nDT_WeaponCSBase__m_flPostponeFireReadyTime() > serverTime)
        return false;

    return true;
}

void EntityCache::cacheEntities() {
    EntityCache::localPlayer = Interfaces::entityList->getClientEntity(Interfaces::engine->getLocalPlayer());
    std::lock_guard<std::mutex> lock(entityCacheLock);
    entityCache.clear();
    if (!localPlayer)
        return;
    for (int i = 1; i < Interfaces::entityList->getHighestEntityIndex(); i++) {
        Entity* e = (Entity*)Interfaces::entityList->getClientEntity(i);
        if (e) {
            if (!e->dormant()) {
                entityCache.push_back(CachedEntity(e));
                
                if (e->clientClass()->m_ClassID == ClassId::CEnvTonemapController)
                    Visuals::nightmode(e);
            }
        }
    }
}