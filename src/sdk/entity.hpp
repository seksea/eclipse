#pragma once
#include <cstdint>
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
    VFUNC(bool, setupBones, 10, (matrix3x4_t* boneMatrix, int maxBones, int boneMask, float curTime = 0), (renderable(), boneMatrix, maxBones, boneMask, curTime))

    VFUNC(Vector&, origin, 12, (), (this))
};