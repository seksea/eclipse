#include <SDL2/SDL.h>

namespace Menu {
    inline bool initialised = false;
    inline bool menuOpen = true;
    inline int curTab = 0;

    void onPollEvent(SDL_Event* event, const int result);
    void onSwapWindow(SDL_Window* window);
}