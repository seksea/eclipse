#include "entity.hpp"

void EntityCache::cacheEntities() {
    std::lock_guard<std::mutex> lock(entityCacheLock);
    entityCache.clear();
    for (int i = 1; i < Interfaces::entityList->getHighestEntityIndex(); i++) {
        Entity* e = (Entity*)Interfaces::entityList->getClientEntity(i);
        if (e) {
            if (!e->dormant()) {
                entityCache.push_back(CachedEntity(e));
            }
        }
    }
}