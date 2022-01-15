#pragma once
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <string>

#include "sdk/interfaces/icvar.hpp"
#include "sdk/interfaces/ivengineclient.hpp"
#include "sdk/interfaces/icliententitylist.hpp"
#include "sdk/interfaces/ibaseclientdll.hpp"
#include "sdk/interfaces/ivmodelrender.hpp"
#include "sdk/interfaces/imaterialsystem.hpp"
#include "sdk/interfaces/panorama.hpp"
#include "sdk/interfaces/igameevent.hpp"
#include "sdk/interfaces/iprediction.hpp"

#include "sdk/interfaces/globals.hpp"
#include "sdk/interfaces/iclientmode.hpp"
#include "sdk/interfaces/viewrenderbeams.hpp"

#include "util/log.hpp"


namespace Interfaces {
    inline ICvar* cvar;
    inline IVEngineClient* engine;
    inline IBaseClientDLL* client;
    inline IClientEntityList* entityList;
    inline IVModelRender* modelRender;
    inline IVModelInfo* modelInfo;
    inline IMaterialSystem* materialSystem;
    inline StudioRender* studioRender;
    inline IPanoramaUIEngine* panorama;
    inline class IEngineSound* sound;
    inline IGameEventManager2* eventManager;
    inline IPrediction* prediction;
    inline IGameMovement* movement;

    inline GlobalVars* globals;
    inline IClientMode* clientMode;
    inline ViewRenderBeams* renderBeams;

    inline int* predictionSeed = 0;
    inline IMoveHelper* moveHelper;
    inline CMoveData* moveData;
    typedef void (*RestoreEntityToPredictedFrame)(void*, int, int);
    inline RestoreEntityToPredictedFrame restoreEntityToPredictedFrame;

    void init();

    typedef void* (*InstantiateInterfaceFn)();
    // https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/public/tier1/interface.h#L72
    class InterfaceReg {
    public:
        InstantiateInterfaceFn m_CreateFn;
        const char* m_pName;
        InterfaceReg* m_pNext;
    };
    template <typename T>
    T* getInterface(const char* file, const char* name, bool includeVersion = false) {
	    void* lib = dlopen(file, RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL);
        if (lib) {
            // https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/tier1/interface.cpp#L46
            InterfaceReg* interfaceReg = *reinterpret_cast<InterfaceReg**>(dlsym(lib, "s_pInterfaceRegs"));
            dlclose(lib);

            // loop through each interface in interfaceReg linked list
            for (InterfaceReg* cur = interfaceReg; cur; cur = cur->m_pNext) {
                // If current interface equals input name without the 3 version numbers so if an interface version changes we dont have to care
                if ((strstr(cur->m_pName, name) && strlen(cur->m_pName)-3 == strlen(name)) || 
                    (includeVersion && (strstr(cur->m_pName, name) && strlen(cur->m_pName) == strlen(name)))) {
                    T* iface = reinterpret_cast<T*>(cur->m_CreateFn());
                    LOG(" %s (%s) %lx", name, cur->m_pName, (uintptr_t)iface);
                    return iface;
                }
            }
        }
        ERR(" Failed to find interface %s in %s", name, file);
        dlclose(lib);
        return nullptr;
    }
}