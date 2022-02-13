#include <algorithm>
#include <unistd.h>

#include "hooks.hpp"
#include "util/log.hpp"
#include "util/memory.hpp"
#include "interfaces.hpp"
#include "sdk/entity.hpp"
#include "menu/menu.hpp"
#include "menu/config.hpp"
#include "features/lua.hpp"
#include "features/chams.hpp"
#include "features/backtrack.hpp"
#include "features/movement.hpp"
#include "features/legitbot.hpp"
#include "features/skinchanger.hpp"
#include "features/glow.hpp"

namespace Hooks {
    void init() {
        SDL::initSDL();

        while (!Menu::initialised)
            usleep(500000);

        LOG(" Hooking CreateMove...");
        CreateMove::original = (CreateMove::func)Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::hook, 25);
        LOG(" Hooking DME...");
        DrawModelExecute::original = (DrawModelExecute::func)Memory::VMT::hook(Interfaces::modelRender, (void*)DrawModelExecute::hook, 21);
        LOG(" Hooking FrameStageNotify...");
        FrameStageNotify::original = (FrameStageNotify::func)Memory::VMT::hook(Interfaces::client, (void*)FrameStageNotify::hook, 37);
        LOG(" Hooking EmitSound...");
        EmitSound::original = (EmitSound::func)Memory::VMT::hook(Interfaces::sound, (void*)EmitSound::hook, 6);
        LOG(" Hooking DoPostScreenEffects...");
        DoPostScreenEffects::original = (DoPostScreenEffects::func)Memory::VMT::hook(Interfaces::clientMode, (void*)DoPostScreenEffects::hook, 45);

        eventListener = new EventListener;
    }

    void unload() {
        SDL::unloadSDL();
        LOG(" Unhooking CreateMove...");
        Memory::VMT::hook(Interfaces::clientMode, (void*)CreateMove::original, 25);
        LOG(" Unhooking DME...");
        Memory::VMT::hook(Interfaces::modelRender, (void*)DrawModelExecute::original, 21);
        LOG(" Unhooking FrameStageNotify...");
        Memory::VMT::hook(Interfaces::client, (void*)FrameStageNotify::original, 37);
        LOG(" Unhooking EmitSound...");
        Memory::VMT::hook(Interfaces::sound, (void*)EmitSound::original, 6);
        LOG(" Unhooking DoPostScreenEffects...");
        Memory::VMT::hook(Interfaces::clientMode, (void*)DoPostScreenEffects::original, 45);

        delete eventListener;
    }

    bool CreateMove::hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd) {
        INFO("started createmove hook.");
        bool origReturn = CreateMove::original(thisptr, flInputSampleTime, cmd);

        if (!cmd || !cmd->commandnumber) {
            INFO("ended createmove hook.");
            return origReturn;
        }

        if (cmd->buttons & IN_SCORE && cmd->tickcount % 32 == 1 && CONFIGBOOL("rank reveal")) {
            Interfaces::client->dispatchUserMessage(50, 0, 0, nullptr);
        }

        Backtrack::store(cmd);
        Backtrack::run(cmd);

        Legitbot::run(cmd);

        Movement::bunnyhop(cmd);

        Lua::curCmd = cmd;
        Lua::handleHook("createMove", *cmd);

        cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
        cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
        cmd->upmove = std::clamp(cmd->upmove, -320.0f, 320.0f);

        cmd->viewangles.y = fmod(cmd->viewangles.y + 180.f, 360.f) - 180.f; // normalize y before clamping
        
        // clamp to avoid untrusted ban
        cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
        cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
        cmd->viewangles.z = 0.0f;

        INFO("ended createmove hook.");
        return false;
    }

    void DrawModelExecute::hook(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld) {
        INFO("started DME hook.");
        Chams::doChams(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
        INFO("ended DME hook.");
    }

    std::vector<std::pair<int, int>> custom_glow_entities;
    void FrameStageNotify::hook(void* thisptr, FrameStage stage) {
        INFO("started FSN %i hook.", stage);
        switch (stage) {
            case FRAME_NET_UPDATE_POSTDATAUPDATE_START: {
                SkinChanger::run();
                break;
            }
            case FRAME_NET_UPDATE_POSTDATAUPDATE_END: {
                Visuals::skyboxChanger();
                break;
            }
            case FRAME_RENDER_END: {
                storedViewMatrix = Interfaces::engine->worldToScreenMatrix();
                EntityCache::cacheEntities();
                break;
            }
        }
        Lua::handleHook("frameStageNotify", stage);
        INFO("ended FSN %i hook.", stage);
        return original(thisptr, stage);
    }

    void EmitSound::hook(void* thisptr, void*& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, void* iSoundLevel, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, void*& params) {
        INFO("started emitSound hook.");
        if (strstr(pSoundEntry, "UIPanorama.popup_accept_match_beep") && CONFIGBOOL("auto accept")) {
            IUIPanel* root = Interfaces::panorama->getRoot();
            if (root)
                Interfaces::panorama->AccessUIEngine()->RunScript(root, "$.DispatchEvent(\"MatchAssistedAccept\");", "panorama/layout/base.xml", 8, 10, false);
            INFO("ended emitSound hook.");
            return;
        }

        if(Prediction::inPrediction && iEntIndex == EntityCache::localPlayer->index()) {
            INFO("ended emitSound hook.");
            return;
        }
        INFO("ended emitSound hook.");
        original(thisptr, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, iSoundLevel, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, params);
    }

    EventListener::EventListener() {
        Interfaces::eventManager->addListener(this, "player_hurt", false);
        Interfaces::eventManager->addListener(this, "player_death", false);
        Interfaces::eventManager->addListener(this, "bullet_impact", false);
    }

    EventListener::~EventListener() {
        Interfaces::eventManager->removeListener(this);
    }

    void EventListener::fireGameEvent(IGameEvent *event) {
        INFO("started fireGameEvent hook.");
        Lua::handleHook("fireEvent", Lua::LuaGameEvent(event));
        INFO("ended EmitSound hook.");
    }

    int EventListener::getEventDebugID() {
        return EVENT_DEBUG_ID_INIT;
    }

    void DoPostScreenEffects::hook(void* thisptr, void* param) {
        INFO("started doPostScreenEffects hook.");
        Glow::draw();
        INFO("ended doPostScreenEffects hook.");
        original(thisptr, param);
    }
}