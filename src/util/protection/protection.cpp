#include "protection.hpp"
#include "../log.hpp"
#include "../../interfaces.hpp"

#include <cpuid.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <sys/signal.h>

namespace Protection {
    uint64_t __attribute__ ((always_inline)) gethwid() {
    unsigned int a, b, model1, model2;
    __get_cpuid(0, &a, &b, &model1, &model2);

    uint64_t model164 = model1;
    
    return ((model164 << 32) +
            model2 + a + b) ^ 0x4e7a8f34c219b3ac;
    }

    bool __attribute__ ((always_inline)) validateHardware() {
        switch (gethwid()) {
            case 736283937490423834: strcpy(username, "sekc"); return true; // sekc
            case 736283937490423839: strcpy(username, "johannes"); return true; // johannes
            case 2458960287626846314: strcpy(username, "vampur"); return true; // vampur
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