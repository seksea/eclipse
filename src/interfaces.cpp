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
        cvar = getInterface<ICvar>("./bin/linux64/materialsystem_client.so", "VEngineCvar");

        engine = getInterface<IVEngineClient>("./bin/linux64/engine_client.so", "VEngineClient");
        cvar->ConsoleColorPrintf({50, 200, 255}, "Got interface %s at %lx", "pog", (uintptr_t)engine);
        client = getInterface<IBaseClientDLL>("./csgo/bin/linux64/client_client.so", "VClient");
        entityList = getInterface<IClientEntityList>("./csgo/bin/linux64/client_client.so", "VClientEntityList");

        /* Get IClientMode */
        uintptr_t HudProcessInput = reinterpret_cast<uintptr_t>(Memory::getVTable(client)[10]);
        typedef IClientMode* (*GetClientMode)();
        GetClientMode getClientMode = reinterpret_cast<GetClientMode>(Memory::getAbsoluteAddress(HudProcessInput + 11, 1, 5));
        clientMode = getClientMode();
        LOG(" ClientMode %lx", (uintptr_t)clientMode);
        LOG("Initialised interfaces!");
    }
}