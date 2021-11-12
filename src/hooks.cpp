#include "hooks.hpp"

namespace Hooks {
    void init() {
        SDL::initSDL();
    }

    void unload() {
        SDL::unloadSDL();
    }
}