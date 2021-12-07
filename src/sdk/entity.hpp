#pragma once
#include <cstdint>
#include <vector>
#include <mutex>
#include "math.hpp"
#include "../util/memory.hpp"
#include "../interfaces.hpp"
#include "netvars.hpp"

#include "allnetvars.hpp"
class ICollideable {
public:
	virtual void pad0();
	virtual const Vector& OBBMins() const;
	virtual const Vector& OBBMaxs() const;
};

namespace EntityCache {
	inline Entity* localPlayer;
}

class Entity {
    public:
	ALL_NETVARS;

	void* networkable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 16);
	}

	void* renderable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x8);
	}

    VFUNC(ClientClass*, clientClass, 2, (), (networkable()))
    VFUNC(bool, dormant, 9, (), (networkable()))
    VFUNC(int, index, 10, (), (networkable()))

    VFUNC(bool, shouldDraw, 5, (), (renderable()))
    VFUNC(bool, setupBones, 13, (matrix3x4_t* boneMatrix, int maxBones, int boneMask, float curTime = 0), (renderable(), boneMatrix, maxBones, boneMask, curTime))

    VFUNC(Vector&, origin, 12, (), (this))

	bool teammate() {
		return this->nDT_BaseEntity__m_iTeamNum() == EntityCache::localPlayer->nDT_BaseEntity__m_iTeamNum();
	}

	const matrix3x4_t coordinateFrame() {
		return *(matrix3x4_t*)((uintptr_t)this + 0x518);
	}
};

namespace EntityCache {
	struct CachedEntity {
		int index;
		int health;
		bool teammate;
		Vector origin;
		int classID;
		PlayerInfo info;
		ImVec4 boundingBox;
		CachedEntity(Entity* e) {
			this->index = e->index();
			this->health = e->nDT_BasePlayer__m_iHealth();
			this->teammate = e->teammate();
			this->origin = e->origin();
			this->classID = e->clientClass()->m_ClassID;
			Interfaces::engine->getPlayerInfo(this->index, this->info);
			boundingBox = getBoundingBox(e);
		}
	};
	
	inline std::vector<CachedEntity> entityCache;
	inline std::mutex entityCacheLock;

	void cacheEntities();
}