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
        cvar = getInterface<ICvar>("./bin/linux64/materialsystem_client.so", "VEngineCvar");
        Log::init();

        engine = getInterface<IVEngineClient>("./bin/linux64/engine_client.so", "VEngineClient");
        client = getInterface<IBaseClientDLL>("./csgo/bin/linux64/client_client.so", "VClient");
        entityList = getInterface<IClientEntityList>("./csgo/bin/linux64/client_client.so", "VClientEntityList");
        modelRender = getInterface<IVModelRender>("./bin/linux64/engine_client.so", "VEngineModel");
        modelInfo = getInterface<IVModelInfo>("./bin/linux64/engine_client.so", "VModelInfoClient");
        materialSystem = getInterface<IMaterialSystem>("./bin/linux64/materialsystem_client.so", "VMaterialSystem");
        studioRender = getInterface<StudioRender>("./bin/linux64/studiorender_client.so", "VStudioRender");
        panorama = getInterface<IPanoramaUIEngine>("./bin/linux64/panorama_gl_client.so", "PanoramaUIEngine");
        sound = getInterface<IEngineSound>("./bin/linux64/engine_client.so", "IEngineSoundClient");

        /* Get IClientMode */
        uintptr_t HudProcessInput = reinterpret_cast<uintptr_t>(Memory::getVTable(client)[10]);
        typedef IClientMode* (*GetClientMode)();
        GetClientMode getClientMode = reinterpret_cast<GetClientMode>(Memory::getAbsoluteAddress(HudProcessInput + 11, 1, 5));
        clientMode = getClientMode();
        LOG(" ClientMode %lx", (uintptr_t)clientMode);

        /* Get Globals */
        uintptr_t hudUpdate = reinterpret_cast<uintptr_t>(Memory::getVTable(client)[11]);
        globals = *reinterpret_cast<GlobalVars**>(Memory::getAbsoluteAddress(hudUpdate + 13, 3, 7));
        LOG(" Globals %lx", (uintptr_t)globals);

        /* Get renderBeams */
        renderBeams = **Memory::relativeToAbsolute<ViewRenderBeams***>(Memory::patternScan("/client_client.so", "4C 89 F6 4C 8B 25 ? ? ? ? 48 8D 05") + 6); // Credit: danielkrupinski
        LOG(" renderBeams %lx", (uintptr_t)renderBeams);

        /* Get panorama panel array (credit LWSS' skeletux project) */
        uintptr_t IsValidPanelPointer = reinterpret_cast<uintptr_t>(Memory::getVTable(Interfaces::panorama->AccessUIEngine())[37]);
        int32_t offset = *(unsigned int*)(IsValidPanelPointer + 4);
        panelArray = *(PanelArray**) ( ((uintptr_t)Interfaces::panorama->AccessUIEngine()) + offset + 8);
        
        LOG("Initialised interfaces!");
    }
}