#include <algorithm>
#include <unistd.h>

#include "hooks.hpp"
#include "util/log.hpp"
#include "util/memory.hpp"
#include "interfaces.hpp"
#include "sdk/entity.hpp"
#include "menu/menu.hpp"
#include "features/lua.hpp"
#include "features/chams.hpp"
#include "features/backtrack.hpp"

namespace Hooks {
    void init() {
        SDL::initSDL();

        while (!Menu::initialised)
            usleep(500000);

        LOG(" Hooking CreateMove...");
        CreateMove::original = (CreateMove::func)Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::hook, 25);
        LOG(" Hooking DME...");
        DrawModelExecute::original = (DrawModelExecute::func)Memory::VMT::hook(Interfaces::modelRender, (void*)DrawModelExecute::hook, 21);
    }

    void unload() {
        SDL::unloadSDL();
        LOG(" Unhooking CreateMove...");
        Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::original, 25);
        LOG(" Unhooking DME...");
        Memory::VMT::hook(Interfaces::modelRender, (void*)DrawModelExecute::original, 21);
    }

    bool CreateMove::hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd) {
        bool origReturn = CreateMove::original(thisptr, flInputSampleTime, cmd);

        EntityCache::cacheEntities();

        Backtrack::store(cmd);
        Backtrack::run(cmd);

        Lua::curCmd = cmd;
        Lua::handleHook("createMove");

        cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
        cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
        cmd->upmove = std::clamp(cmd->upmove, -320.0f, 320.0f);

        cmd->viewangles.y = fmod(cmd->viewangles.y + 180.f, 360.f) - 180.f; // normalize y before clamping
        
        // clamp to avoid untrusted ban
        cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
        cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
        cmd->viewangles.z = 0.0f;

        return origReturn;
    }

    void Hooks::DrawModelExecute::hook(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld) {
        Chams::doChams(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
    }
}