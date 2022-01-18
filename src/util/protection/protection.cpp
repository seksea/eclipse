#include "protection.hpp"

#include <SDL2/SDL.h>
#include <unistd.h>
#include <sys/signal.h>

#define SEGFAULT() int* a = nullptr; *a = 1

namespace Protection {
    unsigned int login(const char* username, const char* passwd) {
        // send username, password, hwid, and user account name to server and receive token in return
    }

    bool validateToken(unsigned int token) {
        return token > 5; // TODO: decrypt token with pub key and check valid
    }

    void protect() {
        if (validateToken(LoginInfo::token)) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "eclipse error", "eclipse failed to pass protection checks, this has been reported to the server.", nullptr);
            SEGFAULT();
        }
    }
}