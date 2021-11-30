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
}