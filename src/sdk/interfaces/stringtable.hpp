#pragma once
#include "../../util/memory.hpp"

class INetworkStringTable {
public:
    VFUNC(int, addString, 9, (bool isServer, const char* name, int length = -1, const void *userdata = 0), (this, isServer, name, length, userdata))
};

class CNetworkStringTableContainer {
public:
    VFUNC(INetworkStringTable*, findTable, 4, (const char* name), (this, name))
};