#include "legitbot.hpp"
#include "backtrack.hpp"
#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"
#include "../menu/config.hpp"

namespace Legitbot {
    void run(CUserCmd* cmd) {
        if (isKeyBinderPressed(&CONFIGBIND("legitbot key"))) {
            float closestBoneDelta = FLT_MAX;
            QAngle angleToClosestBone = { 0, 0, 0 };
            for (Backtrack::Tick tick : Backtrack::ticks) {
                for (std::pair<int, Backtrack::Player> p : tick.players) {
                    if (!Backtrack::isRecordValid(p.second.simTime))
                        continue;
                    Entity* e = Interfaces::entityList->getClientEntity(p.first);
                    if (!e || e == EntityCache::localPlayer || 
                        e->teammate() || e->nDT_BasePlayer__m_iHealth() == 0 || !e->visCheck())
                        continue;
                    
                    Vector targetBonePos = Vector(p.second.boneMatrix[8][0][3], p.second.boneMatrix[8][1][3], p.second.boneMatrix[8][2][3]);

                    QAngle angleToCurrentBone = calcAngle(EntityCache::localPlayer->eyepos(), targetBonePos) -
                                                    cmd->viewangles - (EntityCache::localPlayer->nDT_Local__m_aimPunchAngle() * 2);
                    
                    angleToCurrentBone.y = fmod(angleToCurrentBone.y + cmd->viewangles.y + 180.f, 360.f) - 180.f - cmd->viewangles.y;

                    if (angleToCurrentBone.Length() < closestBoneDelta) {
                        closestBoneDelta = angleToCurrentBone.Length();
                        angleToClosestBone = angleToCurrentBone;
                    }
                }
            }
            if (closestBoneDelta < CONFIGFLOAT("legitbot fov"))
                cmd->viewangles += angleToClosestBone / (1 + CONFIGFLOAT("legitbot smoothing"));
                Interfaces::engine->setViewAngles(cmd->viewangles);
        }
    }
}