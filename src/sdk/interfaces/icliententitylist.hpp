#pragma once
#include <cstdint>

#include "../../util/memory.hpp"

class IClientEntityList {
public:
    // NOTE: This function is only a convenience wrapper.
    // It returns GetClientNetworkable( entnum )->GetIClientEntity().   
    VFUNC(void*, getClientEntity, 3, (int i), (this, i))
    VFUNC(void*, getClientEntityFromHandle, 4, (void* handle), (this, handle))
    VFUNC(int, getHighestEntityIndex, 6, (), (this))
};