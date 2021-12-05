#pragma once
#include <cstdint>

#include "../../util/memory.hpp"

class Entity;

class IClientEntityList {
public:
    // NOTE: This function is only a convenience wrapper.
    // It returns GetClientNetworkable( entnum )->GetIClientEntity().   
    VFUNC(Entity*, getClientEntity, 3, (int i), (this, i))
    VFUNC(Entity*, getClientEntityFromHandle, 4, (void* handle), (this, handle))
    VFUNC(int, getHighestEntityIndex, 6, (), (this))
};