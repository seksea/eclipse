#include <cstdint>
#include <string>

#include "interfaces.hpp"
#include "util/log.hpp"

template <typename T>
static constexpr auto relativeToAbsolute(std::uintptr_t address) noexcept {
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

namespace Interfaces {
    void init() {
        LOG("Initialising interfaces...");
        engine = getInterface<IVEngineClient>("./bin/linux64/engine_client.so", "VEngineClient");
        LOG("Initialised interfaces!");
    }
}