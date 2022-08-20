#include "legitbot.hpp"
#include "backtrack.hpp"
#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"
#include "../menu/config.hpp"
#include "../menu/keybinders.hpp"
#include "ragebot.hpp"

Entity* findPlayerThatRayHits(Vector start, Vector end, Trace* traceToPlayer) {
    Ray ray;
    ray.Init(start, end);
    TraceFilter filter;
    filter.pSkip = EntityCache::localPlayer;
                       // (MASK_SHOT_HULL | CONTENTS_HITBOX)
    Interfaces::trace->traceRay(ray, 1174421515, &filter, traceToPlayer);

    return traceToPlayer->m_pEntityHit;
}

QAngle lastViewangles;

namespace Legitbot {
    bool shouldHit(QAngle viewAngles, int minChance) {
        if (!EntityCache::localPlayer || !EntityCache::localPlayer->canShoot()) return false;
        Entity* activeWeapon = Interfaces::entityList->getClientEntity(
             EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
        if (!activeWeapon) return false;

        Vector endPos;
        Trace traceToPlayer;
        int hitChance = 0;

        float spread = RAD2DEG(activeWeapon->getInaccuracy() + activeWeapon->getSpread());
        for (int i = 0; i < 100 && hitChance < minChance; i++) {
            QAngle randomSpreadAngle = {randFloat(0, spread) - (spread / 2), randFloat(0, spread) - (spread / 2),
                                        randFloat(0, spread) - (spread / 2)};

            angleVectors(viewAngles + randomSpreadAngle, endPos);

            endPos = EntityCache::localPlayer->eyepos() + (endPos *= 4096.f);

            Entity* e = findPlayerThatRayHits(EntityCache::localPlayer->eyepos(), endPos, &traceToPlayer);
            if (!e || e == EntityCache::localPlayer || e->teammate() || e->nDT_BasePlayer__m_iHealth() == 0 || !e->visCheck() ||
                e->nDT_CSPlayer__m_bGunGameImmunity())
                continue;

            hitChance++;
        }

        return hitChance >= minChance;
    }

    void aimbot(CUserCmd* cmd) {
        float fov = CONFIGFLOAT("default fov");
        float smoothing = CONFIGFLOAT("default smoothing");
        int hitboxes = CONFIGINT("default hitboxes");
        bool accountRecoil = true;
        bool distanceFov = CONFIGBOOL("default distance fov");
        bool linearSmoothing = CONFIGBOOL("default linear");

        if (!EntityCache::localPlayer || !EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon())
            return;
        
        Entity* activeWeapon = Interfaces::entityList->getClientEntity(EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
        if (!activeWeapon || activeWeapon->clientClass()->m_ClassID == CKnife)
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
                //accountRecoil = false;

                if (!CONFIGBOOL("pistol override"))
                    break;
                fov = CONFIGFLOAT("pistol fov");
                smoothing = CONFIGFLOAT("pistol smoothing");
                hitboxes = CONFIGINT("pistol hitboxes");
                distanceFov = CONFIGBOOL("pistol distance fov");
                linearSmoothing = CONFIGBOOL("pistol linear");
                break;
            }
            case WEAPON_DEAGLE:
            case WEAPON_REVOLVER: {
                accountRecoil = false; // we want the rest of the pistols besides deagle and revolvo to account for recoil

                if (!CONFIGBOOL("heavy pistol override"))
                    break;
                fov = CONFIGFLOAT("heavy pistol fov");
                smoothing = CONFIGFLOAT("heavy pistol smoothing");
                hitboxes = CONFIGINT("heavy pistol hitboxes");
                distanceFov = CONFIGBOOL("heavy pistol distance fov");
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
                hitboxes = CONFIGINT("rifle hitboxes");
                distanceFov = CONFIGBOOL("rifle distance fov");
                linearSmoothing = CONFIGBOOL("rifle linear");
                break;
            }
            case WEAPON_SSG08: {
                accountRecoil = false;
                
                if (!CONFIGBOOL("scout override"))
                    break;
                fov = CONFIGFLOAT("scout fov");
                smoothing = CONFIGFLOAT("scout smoothing");
                hitboxes = CONFIGINT("scout hitboxes");
                distanceFov = CONFIGBOOL("scout distance fov");
                linearSmoothing = CONFIGBOOL("scout linear");
                break;
            }
            case WEAPON_AWP: {
                accountRecoil = false;
                
                if (!CONFIGBOOL("AWP override"))
                    break;
                fov = CONFIGFLOAT("AWP fov");
                smoothing = CONFIGFLOAT("AWP smoothing");
                hitboxes = CONFIGINT("AWP hitboxes");
                distanceFov = CONFIGBOOL("AWP distance fov");
                linearSmoothing = CONFIGBOOL("AWP linear");
                break;
            }
        }

        if (isKeyBinderPressed(&CONFIGBIND("legitbot key")) || (CONFIGBIND("legitbot key").key == -1 /*mouse1*/ && cmd->buttons & IN_ATTACK)) {
            float closestBoneDelta = FLT_MAX;
            QAngle angleToClosestBone = { 0, 0, 0 };
            for (Backtrack::Tick tick : Backtrack::ticks) {
                for (std::pair<int, Backtrack::Player> p : tick.players) {
                    if (!Backtrack::isRecordValid(p.second.simTime))
                        continue;
                    Entity* e = Interfaces::entityList->getClientEntity(p.first);
                    if (!e || e == EntityCache::localPlayer || 
                        e->teammate() || e->nDT_BasePlayer__m_iHealth() == 0 || (!CONFIGBOOL("autowall") && !e->visCheck()) || e->nDT_CSPlayer__m_bGunGameImmunity())
                        continue;

                    const int bones[5] = {8, 7, 6, 5, 3}; // TODO: make more sekc
                    for (int i = 0; i < 5; i++) {
                        int bone;
                        if(hitboxes & 1 << i)
                            bone = bones[i];
                        else
                            continue;
                        Vector targetBonePos = Vector(p.second.boneMatrix[bone][0][3], p.second.boneMatrix[bone][1][3],
                                                      p.second.boneMatrix[bone][2][3]);

                        QAngle angleToCurrentBone = calcAngle(EntityCache::localPlayer->eyepos(), targetBonePos) -
                             cmd->viewangles -
                             (accountRecoil ? (EntityCache::localPlayer->nDT_Local__m_aimPunchAngle() * 2) : QAngle(0, 0, 0));

                        // make sane ...
                        sanitizeAngles(angleToCurrentBone);

                        if (angleToCurrentBone.Length() < closestBoneDelta) {
                            if (distanceFov) {
                                const float meterToInch = 39.37008f;
                                const float minDist = 5.f * meterToInch;
                                float curDist =
                                     std::max((EntityCache::localPlayer->eyepos() - targetBonePos).Length(), minDist);
                                if (angleToCurrentBone.Length() > fov * (minDist / curDist)) continue;
                            }
                            if ((smoothing == 0 &&
                                 !Ragebot::shouldWallbang(
                                      cmd->viewangles + angleToCurrentBone +
                                           EntityCache::localPlayer
                                                     ->nDT_Local__m_aimPunchAngle() *
                                                2, 0,
                                      CONFIGINT("mindmg"))) 
                                ||
                                (CONFIGBOOL("autowall") &&
                                 !Ragebot::shouldWallbang(
                                      cmd->viewangles + angleToCurrentBone +
                                           EntityCache::localPlayer
                                                     ->nDT_Local__m_aimPunchAngle() *
                                                2,
                                      0)))
                                continue;
                            closestBoneDelta = angleToCurrentBone.Length();
                            angleToClosestBone = angleToCurrentBone;
                        }
                    }
                }
                if(smoothing == 0 && CONFIGINT("hitchance") > 0) break;
            }
            if (closestBoneDelta < fov) {
                if (smoothing == 0) {  // if smoothing is 0 don't do smoothing calculations
                    if (CONFIGINT("hitchance") > 0 &&                                   // J
                        ((!CONFIGBOOL("autowall") &&                                    // E
                          !(shouldHit(                                                  // S
                                 cmd->viewangles + angleToClosestBone +                 // U
                                      EntityCache::localPlayer                          // S
                                                ->nDT_Local__m_aimPunchAngle() *    
                                           2,                                           // C
                                 CONFIGINT("hitchance")) &&                             // H
                            Ragebot::shouldWallbang(                                    // R
                                 cmd->viewangles + angleToClosestBone +                 // I
                                      EntityCache::localPlayer                          // S
                                                ->nDT_Local__m_aimPunchAngle() *        // T
                                           2,                                       
                                 CONFIGINT("hitchance"),                                // W
                                 CONFIGINT("mindmg"))))                                 // T
                         ||                                                             // F
                         (CONFIGBOOL("autowall") &&
                          !Ragebot::shouldWallbang(                                     // I
                               cmd->viewangles + angleToClosestBone +                   // S
                                    EntityCache::localPlayer
                                              ->nDT_Local__m_aimPunchAngle() *          // T
                                         2,                                             // H
                               CONFIGINT("hitchance"),                                  // I
                               CONFIGINT("mindmg")))))                                  // S
                        return;
                    
                    if(CONFIGBOOL("silent")){
                        startMovementFix(cmd);
                        cmd->viewangles += angleToClosestBone;
                        endMovementFix(cmd);
                        return;
                    }
                    cmd->viewangles += angleToClosestBone;
                    Interfaces::engine->setViewAngles(cmd->viewangles);
 
                    return;
                }

                // calculate smoothing
                if (!linearSmoothing) {
                    cmd->viewangles += angleToClosestBone / (1 + smoothing);
                }
                else {
                    float coeff = (1.0f - (smoothing/100)) / std::max(1.f, angleToClosestBone.Length()) * 4.f;
                    coeff = std::min(1.f, coeff);

                    cmd->viewangles += angleToClosestBone * coeff;
                }

                Interfaces::engine->setViewAngles(cmd->viewangles);
            }
        }
    }

    void triggerbot(CUserCmd* cmd) {
        if (!EntityCache::localPlayer) return;
        if (!CONFIGBOOL("triggerbot active") ||
            (CONFIGBIND("triggerbot key").key &&
             !isKeyBinderPressed(&CONFIGBIND("triggerbot key"))))
            return;

        auto viewAngles = cmd->viewangles +
             EntityCache::localPlayer->nDT_Local__m_aimPunchAngle() * 2;

        static bool shotLast = false;
        if (CONFIGINT("hitchance") > 0) {
            if ((!CONFIGBOOL("autowall") &&
                 shouldHit(viewAngles, CONFIGINT("hitchance")) &&
                 Ragebot::shouldWallbang(viewAngles, 0,
                                         CONFIGINT("mindmg"))) ||
                (CONFIGBOOL("autowall") &&
                 Ragebot::shouldWallbang(viewAngles, CONFIGINT("hitchance"),
                                         CONFIGINT("mindmg")))) {
                if (!shotLast) {
                    cmd->buttons |= IN_ATTACK;
                    shotLast = true;
                } else {
                    shotLast = false;
                }
            }
        } else {
            if (!CONFIGBOOL("autowall")) {
                Vector endPos;
                Trace traceToPlayer;

                angleVectors(viewAngles, endPos);

                endPos =
                     EntityCache::localPlayer->eyepos() + (endPos *= 4096.f);

                Entity* e = findPlayerThatRayHits(
                     EntityCache::localPlayer->eyepos(), endPos,
                     &traceToPlayer);
                if (!e || e == EntityCache::localPlayer || e->teammate() ||
                    e->nDT_BasePlayer__m_iHealth() == 0 || !e->visCheck() ||
                    e->nDT_CSPlayer__m_bGunGameImmunity() ||
                         !Ragebot::shouldWallbang(viewAngles, 0,
                                                 CONFIGINT("mindmg")))
                    return;
            } else {
                if (!Ragebot::shouldWallbang(viewAngles, 0,
                                             CONFIGINT("mindmg")))
                    return;
            }
            if (!shotLast) {
                cmd->buttons |= IN_ATTACK;
                shotLast = true;
            } else {
                shotLast = false;
            }
        }
    }

    void run(CUserCmd* cmd) {
        aimbot(cmd);
        // avoid kicks when raging in casual
        if(CONFIGBOOL("aimstep")){
            QAngle delta = cmd->viewangles - lastViewangles;
            sanitizeAngles(delta); // make sane 
            if(delta.Length() > 27.f){
                delta.x /= delta.Length();
                delta.y /= delta.Length();
                delta *= 27.f;
                cmd->viewangles = lastViewangles + delta;
            }
        }
        sanitizeAngles(cmd->viewangles);
        lastViewangles = cmd->viewangles;
        triggerbot(cmd);
    }
}