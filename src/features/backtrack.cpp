#include <algorithm>
#include "backtrack.hpp"
#include "../sdk/entity.hpp"
#include "../menu/config.hpp"

namespace Backtrack {
    bool isRecordValid(float simtime) {
        // ty masterlooser
        // https://www.unknowncheats.me/forum/counterstrike-global-offensive/359885-fldeadtime-int.html
        static Convar* sv_maxunlag = Interfaces::cvar->findVar("sv_maxunlag");
        if (simtime < floorf(Interfaces::globals->curtime - sv_maxunlag->getFloat())) return false;

        NetChannel* net = Interfaces::engine->getNetworkChannel();
        if (!net) {
            LOG("couldn't get netchannel...");
            return false;
        }

        static Convar* sv_client_min_interp_ratio = Interfaces::cvar->findVar("sv_client_min_interp_ratio");
        static Convar* sv_client_max_interp_ratio = Interfaces::cvar->findVar("sv_client_max_interp_ratio");
        static Convar* cl_updaterate = Interfaces::cvar->findVar("cl_updaterate");
        static Convar* sv_maxupdaterate = Interfaces::cvar->findVar("sv_maxupdaterate");
        static Convar* cl_interp = Interfaces::cvar->findVar("cl_interp");
        static Convar* cl_interp_ratio = Interfaces::cvar->findVar("cl_interp_ratio");

        float lerp = std::max(cl_interp->getFloat(),
                              ((std::clamp(cl_interp_ratio->getFloat(), sv_client_min_interp_ratio->getFloat(),
                                           sv_client_max_interp_ratio->getFloat()))) /
                                   ((sv_maxupdaterate->getFloat()) ? sv_maxupdaterate->getFloat() : cl_updaterate->getFloat()));

        auto delta = std::clamp(net->getLatency(0) + net->getLatency(1) + lerp, 0.f, sv_maxunlag->getFloat()) -
             (TICKS_TO_TIME(EntityCache::localPlayer->nDT_LocalPlayerExclusive__m_nTickBase()) - simtime);

        return fabsf(delta) <= sv_maxunlag->getFloat();
    }

    void store(CUserCmd* cmd) {
        if (EntityCache::localPlayer && cmd->tickcount) {
            Tick currentTick;
            // Fill current tick with players
            for (int i = 1; i < Interfaces::globals->maxClients; i++) {
                Entity* ent = (Entity*)Interfaces::entityList->getClientEntity(i);
                if (ent) {
                    if (!ent->dormant() && ent->nDT_BasePlayer__m_iHealth() > 0 && ent != EntityCache::localPlayer && !ent->teammate()) {
                        Player p;
                        p.simTime = ent->simtime();
                        if (ent->setupBones(p.boneMatrix, 128, BONE_USED_BY_ANYTHING, Interfaces::globals->curtime)) {
                            currentTick.players.insert({ent->index(), p});
                        }
                    }
                }
            }
            // Set tickcount of current tick (so we can go back to it)
            currentTick.tickCount = cmd->tickcount;
            // Insert current tick into ticks
            ticks.insert(ticks.begin(), currentTick);
            
            // Delete ticks we cant backtrack
            static Convar *sv_maxunlag = Interfaces::cvar->findVar("sv_maxunlag");
            while (ticks.size() > TIME_TO_TICKS(sv_maxunlag->getFloat()) || ticks.size() > TIME_TO_TICKS(CONFIGFLOAT("backtrack")) + 1) 
                ticks.pop_back();
        }
    }

    void run(CUserCmd* cmd) {
        if (EntityCache::localPlayer && cmd->buttons & (1 << 0)) {
            float closestDelta = FLT_MAX; 
            int closestTick = cmd->tickcount;
            QAngle viewAngles = cmd->viewangles;
            viewAngles += EntityCache::localPlayer->nDT_Local__m_aimPunchAngle() * 2;
            for (Tick tick : ticks) {
                for (auto p : tick.players) {
                    Entity* ent = Interfaces::entityList->getClientEntity(p.first);
                    if (ent && !ent->dormant() && ent->nDT_BasePlayer__m_iHealth() > 0 && isRecordValid(p.second.simTime)) { // TODO: add eyepos
                        QAngle angleToCurrentPlayer = calcAngle(
                            EntityCache::localPlayer->eyepos(), 
                            Vector(p.second.boneMatrix[8][0][3], p.second.boneMatrix[8][1][3], p.second.boneMatrix[8][2][3]));
                        
                        angleToCurrentPlayer -= viewAngles;
                        if (angleToCurrentPlayer.y > 180.f) {
                            angleToCurrentPlayer.y -= 360.f;
                        }

                        if (angleToCurrentPlayer.Length() < closestDelta) {
                            closestDelta = angleToCurrentPlayer.Length();
                            closestTick = tick.tickCount;
                        }
                    }
                }
            }
            cmd->tickcount = closestTick;
        }
    }
}  // namespace Backtrack