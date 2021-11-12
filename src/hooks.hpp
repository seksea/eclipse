#include <type_traits>
#include <SDL2/SDL.h>

namespace Hooks {
    void init();
    void unload();

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