#include <type_traits>
#include <SDL2/SDL.h>
#include "interfaces.hpp"

namespace Hooks {
    void init();
    void unload();

    namespace CreateMove {
        using func = bool(*)(void* thisptr, float flInputSampleTime, CUserCmd* cmd);
        inline func original;
        bool hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd);
    }

    namespace DrawModelExecute {
        using func = void(*)(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld);
        inline func original;
        void hook(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld);
    }

    namespace FrameStageNotify {
        using func = void(*)(void* thisptr, FrameStage stage);
        inline func original;
        void hook(void* thisptr, FrameStage stage);
    }
    
    namespace EmitSound {
        using func = void(*)(void* thisptr, void*& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, void* iSoundLevel, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, void*& params);
        inline func original;
        void hook(void* thisptr, void*& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, void* iSoundLevel, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, void*& params);
    }
    
    namespace DoPostScreenEffects {
        using func = void(*)(void* thisptr, void* param);
        inline func original;
        void hook(void* thisptr, void* param);
    }
    
    namespace CanLoadThirdPartyFiles {
        using func = int(*)(void* thisptr);
        inline func original;
        int hook(void* thisptr);
    }
    
    namespace GetUnverifiedFileHashes {
        using func = int(*)(void* thisptr, void* tmp, int count);
        inline func original;
        int hook(void* thisptr, void* tmp, int count);
    }
    
    namespace FindMdl {
        using func = unsigned short(*)(void* thisptr, const char* modelPath);
        inline func original;
        unsigned short hook(void* thisptr, const char* modelPath);
    }

    namespace SDL {
        bool initSDL();
        inline uintptr_t swapWindowAddr;
        inline std::add_pointer_t<void(SDL_Window*)> swapWindow;
        inline uintptr_t pollEventAddr;
        inline std::add_pointer_t<int(SDL_Event*)> pollEvent;
        void SwapWindow(SDL_Window* window);
        int PollEvent(SDL_Event* event);
        bool unloadSDL();
    }

    /* Event Manager */
    class EventListener : public IGameEventListener2 {
    public:
        EventListener();
        ~EventListener();

        void fireGameEvent(IGameEvent* event) override;
        int getEventDebugID() override;
    };
    inline EventListener* eventListener = nullptr;
}