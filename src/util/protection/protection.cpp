#include "protection.hpp"

#include <SDL2/SDL.h>
#include <unistd.h>
#include <sys/signal.h>

#define SEGFAULT() int* a = nullptr; *a = 1

namespace Protection {
    void protect() {
        VMProtectBeginUltra("protect func");
        if (false) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "eclipse error", "eclipse failed to pass protection checks", nullptr);
            SEGFAULT();
        }
        VMProtectEnd();
    }
}