#include <algorithm>

#include "hooks.hpp"
#include "util/log.hpp"
#include "util/memory.hpp"
#include "interfaces.hpp"
#include "features/lua.hpp"

namespace Hooks {
    void init() {
        SDL::initSDL();

        LOG(" Hooking CreateMove...");
        CreateMove::original = (CreateMove::func)Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::hook, 25);
    }

    void unload() {
        LOG(" Unhooking CreateMove...");
        Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::original, 25);
        SDL::unloadSDL();
    }

    bool CreateMove::hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd) {
        bool origReturn = CreateMove::original(thisptr, flInputSampleTime, cmd);

        Lua::curCmd = cmd;
        Lua::handleHook("createMove");

        cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
        cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
        cmd->upmove = std::clamp(cmd->upmove, -320.0f, 320.0f);

        cmd->viewangles.y = fmod(cmd->viewangles.y + 180.f, 360.f) - 180.f; // normalize y before clamping

        cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f); // clamp to avoid untrusted ban
        cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
        cmd->viewangles.z = 0.0f;

        return origReturn;
    }
}