#include "legitbot.hpp"
#include "backtrack.hpp"
#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"
#include "../menu/config.hpp"

namespace Legitbot {
    void run(CUserCmd* cmd) {
        float fov = CONFIGFLOAT("default fov");
        float smoothing = CONFIGFLOAT("default smoothing");
        bool accountRecoil = true;
        bool linearSmoothing = CONFIGBOOL("default linear");

        if (!EntityCache::localPlayer || !EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon())
            return;
        
        Entity* activeWeapon = Interfaces::entityList->getClientEntity(EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
        if (!activeWeapon)
            return;

        switch(activeWeapon->nDT_ScriptCreatedItem__m_iItemDefinitionIndex()) {
            case WEAPON_ELITE:
            case WEAPON_FIVESEVEN:
            case WEAPON_GLOCK:
            case WEAPON_TEC9:
            case WEAPON_HKP2000:
            case WEAPON_P250:
            case WEAPON_USP_SILENCER:
            case WEAPON_CZ75A: {
                accountRecoil = false;

                if (!CONFIGBOOL("pistol override"))
                    break;
                fov = CONFIGFLOAT("pistol fov");
                smoothing = CONFIGFLOAT("pistol smoothing");
                linearSmoothing = CONFIGBOOL("pistol linear");
                break;
            }
            case WEAPON_DEAGLE:
            case WEAPON_REVOLVER: {
                accountRecoil = false;

                if (!CONFIGBOOL("heavy pistol override"))
                    break;
                fov = CONFIGFLOAT("heavy pistol fov");
                smoothing = CONFIGFLOAT("heavy pistol smoothing");
                linearSmoothing = CONFIGBOOL("heavy pistol linear");
                break;
            }
            case WEAPON_AK47:
            case WEAPON_AUG:
            case WEAPON_FAMAS:
            case WEAPON_G3SG1:
            case WEAPON_GALILAR:
            case WEAPON_M4A1:
            case WEAPON_SCAR20:
            case WEAPON_SG556:
            case WEAPON_M4A1_SILENCER: {
                if (!CONFIGBOOL("rifle override"))
                    break;
                fov = CONFIGFLOAT("rifle fov");
                smoothing = CONFIGFLOAT("rifle smoothing");
                linearSmoothing = CONFIGBOOL("rifle linear");
                break;
            }
            case WEAPON_SSG08: {
                accountRecoil = false;
                
                if (!CONFIGBOOL("scout override"))
                    break;
                fov = CONFIGFLOAT("scout fov");
                smoothing = CONFIGFLOAT("scout smoothing");
                linearSmoothing = CONFIGBOOL("scout linear");
                break;
            }
            case WEAPON_AWP: {
                accountRecoil = false;
                
                if (!CONFIGBOOL("AWP override"))
                    break;
                fov = CONFIGFLOAT("AWP fov");
                smoothing = CONFIGFLOAT("AWP smoothing");
                linearSmoothing = CONFIGBOOL("AWP linear");
                break;
            }
        }

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
                                cmd->viewangles - (accountRecoil ? (EntityCache::localPlayer->nDT_Local__m_aimPunchAngle() * 2) : QAngle(0, 0, 0));
                    
                    angleToCurrentBone.y = fmod(angleToCurrentBone.y + cmd->viewangles.y + 180.f, 360.f) - 180.f - cmd->viewangles.y;

                    if (angleToCurrentBone.Length() < closestBoneDelta) {
                        closestBoneDelta = angleToCurrentBone.Length();
                        angleToClosestBone = angleToCurrentBone;
                    }
                }
            }
            if (closestBoneDelta < fov) {
                if (smoothing == 0) { // if smoothing is 0 don't do smoothing calculations
                    cmd->viewangles += angleToClosestBone;
                    Interfaces::engine->setViewAngles(cmd->viewangles);
                    return;
                }
                
                // calculate smoothing
                if (!linearSmoothing) {
                    cmd->viewangles += angleToClosestBone / (1 + smoothing);
                }
                else {
                    float coeff = (1.0f - (smoothing/100)) / angleToClosestBone.Length() * 4.f;
                    coeff = std::min(1.f, coeff);

                    cmd->viewangles += angleToClosestBone * coeff;
                }

                Interfaces::engine->setViewAngles(cmd->viewangles);
            }
        }
    }
}