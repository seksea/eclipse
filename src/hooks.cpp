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

namespace Hooks {
    static Hooks::EventListener* eventListener = nullptr;

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

        delete eventListener;
    }

    bool CreateMove::hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd) {
        bool origReturn = CreateMove::original(thisptr, flInputSampleTime, cmd);

        if (!cmd || !cmd->commandnumber)
            return origReturn;
        

        storedViewMatrix = Interfaces::engine->worldToScreenMatrix();
        EntityCache::cacheEntities();

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

        return origReturn;
    }

    void DrawModelExecute::hook(void* thisptr, void* ctx, const DrawModelState &state, const ModelRenderInfo &pInfo, matrix3x4_t *pCustomBoneToWorld) {
        Chams::doChams(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
    }

    void FrameStageNotify::hook(void* thisptr, FrameStage stage) {
        SkinChanger::run(stage);
        Lua::handleHook("frameStageNotify", stage);
        return original(thisptr, stage);
    }

    void EmitSound::hook(void* thisptr, void*& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, void* iSoundLevel, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, void*& params) {
        if (strstr(pSoundEntry, "UIPanorama.popup_accept_match_beep") && CONFIGBOOL("auto accept")) {
            IUIPanel* root = Interfaces::panorama->getRoot();
            if (root)
                Interfaces::panorama->AccessUIEngine()->RunScript(root, "$.DispatchEvent(\"MatchAssistedAccept\");", "panorama/layout/base.xml", 8, 10, false);
        }
        Hooks::EmitSound::original(thisptr, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, iSoundLevel, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, params);
    }

    EventListener::EventListener() {
        Interfaces::eventManager->AddListener(this, "player_hurt", false);
        Interfaces::eventManager->AddListener(this, "player_death", false);
        Interfaces::eventManager->AddListener(this, "bullet_impact", false);
    }

    EventListener::~EventListener() {
        Interfaces::eventManager->RemoveListener(this);
    }

    void EventListener::fireGameEvent(IGameEvent *event) {
        Entity* attacker = (Entity*)Interfaces::entityList->getClientEntity(Interfaces::engine->getPlayerForUserID(event->GetInt("attacker")));
        Entity* victim = (Entity*)Interfaces::entityList->getClientEntity(Interfaces::engine->getPlayerForUserID(event->GetInt("userid")));
        LOG("event: %s", event->GetName());
    }

    int EventListener::getEventDebugID() {
        return EVENT_DEBUG_ID_INIT;
    }
}