#include "entity.hpp"
#include "classids.hpp"

void EntityCache::cacheEntities() {
    localPlayer = Interfaces::entityList->getClientEntity(Interfaces::engine->getLocalPlayer());
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