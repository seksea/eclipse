#include "protection.hpp"
#include "../log.hpp"
#include "../../interfaces.hpp"

#include <cpuid.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <sys/signal.h>

namespace Protection {
    inline bool validateHardware() {
        unsigned int a, b, model1, model2;
        __get_cpuid(0, &a, &b, &model1, &model2);

        switch (model1) {
            case 1145913699: if (model2 == 1769238117) return true; // sekc
        }

        return false;
    }

    void __attribute__ ((noinline)) protect() {
        if (!validateHardware()) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "eclipse error", "eclipse failed to pass protection checks, this has been reported to the server.", nullptr);
            exit(0);
        }
    }
}