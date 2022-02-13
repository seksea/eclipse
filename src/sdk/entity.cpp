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
                                        // solid|opaque|moveable|ignore nodraw
    Interfaces::trace->traceRay(rayToHead, (0x1 | 0x80 | 0x2000), &filter, &traceToHead);

    Trace traceToUpperSpinal;
    Ray rayToUpperSpinal;
    rayToUpperSpinal.Init(EntityCache::localPlayer->eyepos(), Vector(boneMatrix[6][0][3], boneMatrix[6][1][3], boneMatrix[6][2][3]));
                                                // solid|opaque|moveable|ignore nodraw
    Interfaces::trace->traceRay(rayToUpperSpinal, (0x1 | 0x80 |    0x2000   ), &filter, &traceToUpperSpinal);

    return (traceToHead.m_pEntityHit == this) && (traceToUpperSpinal.m_pEntityHit == this) && 
            !Interfaces::lineGoesThroughSmoke(EntityCache::localPlayer->eyepos(), this->eyepos(), 1);
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