#include <algorithm>
#include "backtrack.hpp"
#include "../sdk/entity.hpp"
#include "../menu/config.hpp"

namespace Backtrack {
    bool isRecordValid(float simtime) {
        // ty masterlooser
        // https://www.unknowncheats.me/forum/counterstrike-global-offensive/359885-fldeadtime-int.html
        static Convar *sv_maxunlag = Interfaces::cvar->findVar("sv_maxunlag");
        return simtime >= floorf(Interfaces::globals->curtime - sv_maxunlag->getFloat());
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
                        p.simTime = ent->nDT_BaseEntity__m_flSimulationTime();
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
                            EntityCache::localPlayer->origin() + Vector(0, 0, (EntityCache::localPlayer->nDT_BasePlayer__m_fFlags() & (1 << 1)) ? 46 : 64), 
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
}