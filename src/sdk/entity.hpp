#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <mutex>
#include "math.hpp"
#include "../util/memory.hpp"
#include "../interfaces.hpp"
#include "netvars.hpp"
#include "../features/visuals.hpp"

#include "allnetvars.hpp"
class ICollideable {
public:
	virtual void pad0();
	virtual Vector& OBBMins();
	virtual Vector& OBBMaxs();
};

namespace EntityCache {
	inline Entity* localPlayer;
}

using WeaponArr = std::array<unsigned long, 48>;

class Entity {
    public:
	ALL_NETVARS;

	NETVAR("DT_BaseCombatCharacter", "m_hMyWeapons", weapons, WeaponArr);
	
	void* networkable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 16);
	}

	void* renderable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x8);
	}

    VFUNC(ClientClass*, clientClass, 2, (), (networkable()))
    VFUNC(void, onPreDataChanged, 4, (int updateType), (networkable(), updateType))
    VFUNC(void, onDataChanged, 5, (int updateType), (networkable(), updateType))
    VFUNC(void, preDataUpdate, 6, (int updateType), (networkable(), updateType))
    VFUNC(void, postDataUpdate, 7, (int updateType), (networkable(), updateType))
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

	const int moveType() {
		return *reinterpret_cast<int*>((uintptr_t)this + Netvars::netvars.at({"DT_BaseEntity", "m_nRenderMode"}).second + 1);
	}
};

namespace EntityCache {
	struct CachedEntity {
		int index;
		Vector origin;
		int classID;
		ImVec4 boundingBox;
		int health;
		bool teammate;
		PlayerInfo info;
		CachedEntity(Entity* e) {
			this->index = e->index();
			this->origin = e->origin();
			this->classID = e->clientClass()->m_ClassID;
			boundingBox = getBoundingBox(e);
			if (this->index <= 64) { // if player
				this->health = e->nDT_BasePlayer__m_iHealth();
				this->teammate = e->teammate();
				Interfaces::engine->getPlayerInfo(this->index, this->info);
			}
		}
	};
	
	inline std::vector<CachedEntity> entityCache;
	inline std::mutex entityCacheLock;

	void cacheEntities();
}