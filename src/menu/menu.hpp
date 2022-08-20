#include <SDL2/SDL.h>
#include "imgui/imgui.h"

namespace Menu {
    inline bool initialised = false;
    inline bool menuOpen = true;
    inline int curTab = 0;
    inline char curConfigLoaded[128] = "";
    
    inline ImFont* menuFont;
    inline ImFont* weaponFont;

    inline ImVec2 windowPos, windowSize;

    void init(SDL_Event* event, const int result, SDL_Window* pWindow);
    void onPollEvent(SDL_Event* event, const int result);
    void render(SDL_Window* window);
}