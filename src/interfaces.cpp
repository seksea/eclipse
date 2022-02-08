#include <cstdint>
#include <string>

#include "interfaces.hpp"
#include "util/log.hpp"
#include "menu/config.hpp"

template <typename T>
static constexpr auto relativeToAbsolute(std::uintptr_t address) noexcept {
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

bool host_IsSecureServerAllowed_hook() {
    return CONFIGBOOL("insecure bypass");
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
        eventManager = getInterface<IGameEventManager2>("./bin/linux64/engine_client.so", "GAMEEVENTSMANAGER002", true);
        prediction = getInterface<IPrediction>("./csgo/bin/linux64/client_client.so", "VClientPrediction001", true);
	    movement = getInterface<IGameMovement>("./csgo/bin/linux64/client_client.so", "GameMovement");
	    trace = getInterface<IEngineTrace>("./bin/linux64/engine_client.so", "EngineTraceClient");

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


        typedef GlowObjectManager* (*GlowObjectManagerFn) (void);
	    glowManager = ((GlowObjectManagerFn)Memory::getAbsoluteAddress(Memory::patternScan("/client_client.so", "E8 ? ? ? ? 48 8B 3D ? ? ? ? BE 01 00 00 00 C7"), 1, 5))();
        LOG(" glowManager %lx", (uintptr_t)glowManager);

        /* Get panorama panel array (credit LWSS' skeletux project) */
        uintptr_t IsValidPanelPointer = reinterpret_cast<uintptr_t>(Memory::getVTable(Interfaces::panorama->AccessUIEngine())[37]);
        int32_t offset = *(unsigned int*)(IsValidPanelPointer + 4);
        panelArray = *(PanelArray**) ( ((uintptr_t)Interfaces::panorama->AccessUIEngine()) + offset + 8);


        predictionSeed = *reinterpret_cast<int **>(Memory::getAbsoluteAddress(Memory::patternScan("/client_client.so", 
                "48 8B 05 ? ? ? ? 8B 38 E8 ? ? ? ? 89 C7"), 3, 7));
        LOG(" predictionSeed %lx", predictionSeed);

        moveHelper = *reinterpret_cast<IMoveHelper **>(Memory::getAbsoluteAddress(Memory::patternScan("/client_client.so", 
                "00 48 89 3D ? ? ? ? C3") + 1, 3, 7));
        LOG(" moveHelper %lx", moveHelper);

        moveData = **reinterpret_cast<CMoveData***>(Memory::getAbsoluteAddress(Memory::patternScan("/client_client.so", 
                "48 8B 0D ? ? ? ? 4C 89 EA"), 3, 7));
        LOG(" moveData %lx", moveData);

        restoreEntityToPredictedFrame = (RestoreEntityToPredictedFrame)Memory::patternScan("/client_client.so",
            "55 48 89 E5 41 57 41 89 D7 41 56 41 55 41 89 F5 41 54 53 48 83 EC 18");
        LOG(" restoreEntityToPredictedFrame %lx", restoreEntityToPredictedFrame);

        lineGoesThroughSmoke = (LineGoesThroughSmoke)Memory::patternScan("/client_client.so", 
                "55 48 89 E5 41 56 41 55 41 54 53 48 83 EC 30 66 0F D6 45 D0");
        LOG(" lineGoesThroughSmoke | %lx", lineGoesThroughSmoke);

        host_IsSecureServerAllowed = (Host_IsSecureServerAllowed)Memory::patternScan("/engine_client.so", "55 48 89 E5 E8 ? ? ? ? 48 8D 35 ? ? ? ? 48 8B 10 48 89 C7 FF 52 58 85 C0 74 13");
        insecure = !host_IsSecureServerAllowed();
        if (insecure) {
            Memory::VMT::detour((char*)(host_IsSecureServerAllowed), (char*)(host_IsSecureServerAllowed_hook));
        }

        LOG("Initialised interfaces!");
    }
}