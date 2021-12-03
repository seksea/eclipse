#include <mutex>
#include "esp.hpp"
#include "../sdk/entity.hpp"

namespace ESP {
    void draw(ImDrawList* drawList) {
		std::lock_guard<std::mutex> lock(EntityCache::entityCacheLock);
        for (auto e : EntityCache::entityCache) {
            if (e.boundingBox.w < 0 || isnan(e.boundingBox.w) ) 
                continue;
            if (e.classID == 1) {
                drawList->AddRect(ImVec2(e.boundingBox.x, e.boundingBox.y), ImVec2(e.boundingBox.z, e.boundingBox.w), ImColor(255, 255, 255));
            }
        }
    }
}