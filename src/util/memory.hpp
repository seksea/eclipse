#pragma once
#include <stdint.h>
#include <cstddef>

namespace VirtualMethod
{
    template <typename T, int index, typename ...Args>
    constexpr T call(void* classBase, Args... args) noexcept
    {
        return (*reinterpret_cast<T(***)(void*, Args...)>(classBase))[index](classBase, args...);
    }
}

#define VFUNC(returnType, name, idx, args, argsRaw) \
returnType name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

#define VFUNC_V(returnType, name, idx, args, argsRaw) VIRTUAL_METHOD(returnType, name, idx + 1, args, argsRaw)


namespace Memory {
    namespace VMT {
        void* hook(void* instance, void* hook, int offset);
    }

    inline void**& getVTable(void* c, size_t offset = 0) {
        return *reinterpret_cast<void***>((size_t)c + offset);
    }

    inline uintptr_t getAbsoluteAddress(uintptr_t ptr, int offset, int size) {
        return ptr + *reinterpret_cast<int32_t*>(ptr + offset) + size;
    };
}