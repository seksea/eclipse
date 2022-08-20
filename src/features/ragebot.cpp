#include "ragebot.hpp"

#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"

static float GetHitgroupDamageMultiplier(HitGroups iHitGroup) {
    switch (iHitGroup) {
        case HitGroups::HITGROUP_HEAD:
            return 4.0f;
        case HitGroups::HITGROUP_CHEST:
        case HitGroups::HITGROUP_LEFTARM:
        case HitGroups::HITGROUP_RIGHTARM:
            return 1.0f;
        case HitGroups::HITGROUP_STOMACH:
            return 1.25f;
        case HitGroups::HITGROUP_LEFTLEG:
        case HitGroups::HITGROUP_RIGHTLEG:
            return 0.75f;
        default:
            return 1.0f;
    }
}

static void ScaleDamage(HitGroups hitgroup, Entity* enemy, float weapon_armor_ratio, float& current_damage) {
    current_damage *= GetHitgroupDamageMultiplier(hitgroup);

    if (enemy->nDT_CSPlayer__m_ArmorValue() > 0) {
        if (hitgroup == HitGroups::HITGROUP_HEAD) {
            if (enemy->nDT_CSPlayer__m_bHasHelmet()) current_damage *= weapon_armor_ratio * 0.5f;
        } else
            current_damage *= weapon_armor_ratio * 0.5f;
    }
}

static bool TraceToExit(Vector& end, Trace* enter_trace, Vector start, Vector dir, Trace* exit_trace) {
    float distance = 0.0f;

    while (distance <= 90.0f) {
        distance += 4.0f;
        Vector retar = dir;
        retar *= distance;
        end = start + retar /*dir * distance*/;

        auto point_contents =
             Interfaces::trace->getPointContents(end, 1174421515 /*MASK_SHOT_HULL | CONTENTS_HITBOX*/, nullptr);

        if (point_contents & 100679691 /*MASK_SHOT_HULL*/ && !(point_contents & 1073741824 /*CONTENTS_HITBOX*/)) continue;

        retar = dir;
        retar *= 4.f;
        auto new_end = end - retar /*(dir * 4.0f)*/;

        Ray ray;
        ray.Init(end, new_end);
        Interfaces::trace->traceRay(ray, 1174421507U /*MASK_SHOT*/, 0, exit_trace);

        if (exit_trace->startsolid && exit_trace->surface.flags & 32768 /*SURF_HITBOX*/) {
            ray.Init(end, start);

            TraceFilter filter;
            filter.pSkip = exit_trace->m_pEntityHit;

            Interfaces::trace->traceRay(ray, 0x600400B, &filter, exit_trace);

            if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid) {
                end = exit_trace->endpos;
                return true;
            }

            continue;
        }

        if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid) {
            if (exit_trace->m_pEntityHit) {
                if (enter_trace->m_pEntityHit &&
                    enter_trace->m_pEntityHit->clientClass() ==
                         EntityCache::localPlayer->clientClass())  // ==
                                                                   // entityList->GetClientEntity(Aimbot::targetAimbot))
                    return true;
            }

            continue;
        }

        if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1)) continue;

        if (exit_trace->plane.normal.Dot(dir) <= 1.0f) {
            auto fraction = exit_trace->fraction * 4.0f;
            retar = dir;
            retar *= fraction;
            end = end - retar /*(dir * fraction)*/;

            return true;
        }
    }

    return false;
}

static bool HandleBulletPenetration(CCSWeaponInfo* weaponInfo, Ragebot::fireBulletData& data) {
    surfacedata_t* enter_surface_data = Interfaces::physics->GetSurfaceData(data.enter_trace.surface.surfaceProps);
    int enter_material = enter_surface_data->game.material;
    float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;

    data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
    data.current_damage *= powf(weaponInfo->GetRangeModifier(), data.trace_length * 0.002f);

    if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f) data.penetrate_count = 0;

    if (data.penetrate_count <= 0) return false;

    Vector dummy;
    Trace trace_exit;

    if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit)) return false;

    surfacedata_t* exit_surface_data = Interfaces::physics->GetSurfaceData(trace_exit.surface.surfaceProps);
    int exit_material = exit_surface_data->game.material;

    float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;

    float final_damage_modifier = 0.16f;
    float combined_penetration_modifier = 0.0f;

    if ((data.enter_trace.contents & 8 /*CONTENTS_GRATE*/) != 0 || enter_material == 89 || enter_material == 71) {
        combined_penetration_modifier = 3.0f;
        final_damage_modifier = 0.05f;
    } else
        combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

    if (enter_material == exit_material) {
        if (exit_material == 87 || exit_material == 85)
            combined_penetration_modifier = 3.0f;
        else if (exit_material == 76)
            combined_penetration_modifier = 2.0f;
    }

    float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
    float v35 = (data.current_damage * final_damage_modifier) +
         v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->GetPenetration()) * 1.25f);
    float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

    thickness *= thickness;
    thickness *= v34;
    thickness /= 24.0f;

    float lost_damage = fmaxf(0.0f, v35 + thickness);

    if (lost_damage > data.current_damage) return false;

    if (lost_damage >= 0.0f) data.current_damage -= lost_damage;

    if (data.current_damage < 1.0f) return false;

    data.src = trace_exit.endpos;
    data.penetrate_count--;

    return true;
}

static void TraceLine(Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, Entity* ignore, Trace* ptr) {
    Ray ray;
    ray.Init(vecAbsStart, vecAbsEnd);
    TraceFilter filter;
    filter.pSkip = ignore;

    Interfaces::trace->traceRay(ray, mask, &filter, ptr);
}

static bool SimulateFireBullet(Entity* pWeapon, bool teamCheck, Ragebot::fireBulletData& data) {
    Entity* localplayer = EntityCache::localPlayer;  //(C_BasePlayer*)
                                                     // entityList->GetClientEntity(engine->GetLocalPlayer());
    CCSWeaponInfo* weaponInfo = pWeapon->getCSWpnData();

    data.penetrate_count = 4;
    data.trace_length = 0.0f;
    data.current_damage = (float)weaponInfo->GetDamage();

    while (data.penetrate_count > 0 && data.current_damage >= 1.0f) {
        data.trace_length_remaining = weaponInfo->GetRange() - data.trace_length;

        Vector retar = data.direction;
        retar *= data.trace_length_remaining;
        Vector end = data.src + retar /*data.direction * data.trace_length_remaining*/;

        // data.enter_trace
        TraceLine(data.src, end, 1174421507U /*MASK_SHOT*/, localplayer, &data.enter_trace);

        Ray ray;
        retar = data.direction;
        retar *= 40.f;
        ray.Init(data.src, end + retar /*data.direction * 40.f*/);

        Interfaces::trace->traceRay(ray, 1174421507U /*MASK_SHOT*/, &data.filter, &data.enter_trace);

        retar = data.direction;
        retar *= 40.f;
        TraceLine(data.src, end + retar /*data.direction * 40.f*/, 1174421507U /*MASK_SHOT*/, localplayer, &data.enter_trace);

        if (data.enter_trace.fraction == 1.0f) break;

        if (data.enter_trace.hitgroup <= HitGroups::HITGROUP_RIGHTLEG &&
            data.enter_trace.hitgroup > HitGroups::HITGROUP_GENERIC) {
            data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
            data.current_damage *= powf(weaponInfo->GetRangeModifier(), data.trace_length * 0.002f);

            Entity* player = (Entity*)data.enter_trace.m_pEntityHit;
            if (teamCheck && player->teammate() /*Entity::IsTeamMate(player, localplayer)*/) return false;

            ScaleDamage(data.enter_trace.hitgroup, player, weaponInfo->GetWeaponArmorRatio(), data.current_damage);

            return true;
        }

        if (!HandleBulletPenetration(weaponInfo, data)) break;
    }

    return false;
}

float Ragebot::GetDamage(const Vector& point, bool teamCheck, fireBulletData& fData) {
    float damage = 0.f;
    Vector dst = point;
    Entity* localplayer = EntityCache::localPlayer;  //(C_BasePlayer*)
                                                     // entityList->GetClientEntity(engine->GetLocalPlayer());
    fireBulletData data;
    data.src = localplayer->eyepos();
    data.filter.pSkip = localplayer;

    QAngle angles = calcAngle(data.src, dst);
    angleVectors(angles, data.direction);

    Vector tmp = data.direction;
    data.direction = tmp.Normalize();

    // Entity* activeWeapon = (C_BaseCombatWeapon*)
    // entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    Entity* activeWeapon =
         Interfaces::entityList->getClientEntity(EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
    if (!activeWeapon) return -1.0f;

    if (SimulateFireBullet(activeWeapon, teamCheck, data)) damage = data.current_damage;

    fData = data;

    return damage;
}

Entity* clipRayToEntity(Vector start, Vector end, Trace* traceToPlayer, Entity* pEntity) {
    Ray ray;
    ray.Init(start, end);
    // TraceFilter filter;
    // filter.pSkip = EntityCache::localPlayer;
    //  (MASK_SHOT_HULL | CONTENTS_HITBOX)
    Interfaces::trace->clipRayToEntity(ray, 1174421515, pEntity, traceToPlayer);

    return traceToPlayer->m_pEntityHit;
}

bool Ragebot::shouldWallbang(QAngle viewAngles, int minChance, int minDamage) {
    // do some awall shit to see if bullet would penetrate and hit with no
    // spread
    Entity* activeWeapon =
         Interfaces::entityList->getClientEntity(EntityCache::localPlayer->nDT_BaseCombatCharacter__m_hActiveWeapon() & 0xfff);
    if (!activeWeapon) return false;
    fireBulletData data;
    data.src = EntityCache::localPlayer->eyepos();
    data.filter.pSkip = EntityCache::localPlayer;
    angleVectors(viewAngles, data.direction);
    Vector tmp = data.direction;
    data.direction = tmp.Normalize();
    if (!SimulateFireBullet(activeWeapon, true, data)) return false;
    if (Entity* e = data.enter_trace.m_pEntityHit; !e || e == EntityCache::localPlayer || e->teammate() ||
        e->nDT_BasePlayer__m_iHealth() == 0 || e->nDT_CSPlayer__m_bGunGameImmunity())
        return false;

    if (data.current_damage < minDamage && data.current_damage < data.enter_trace.m_pEntityHit->nDT_BasePlayer__m_iHealth())
        return false;
    if (minChance == 0) return true;
    // do basic hitchance calculation with clipRayToEntity instead of normal
    // TraceRay so we don't have to awall every bullet in the calc (fps++)
    Vector endPos;
    Trace traceToPlayer;
    int hitChance = 0;

    float spread = RAD2DEG(activeWeapon->getInaccuracy() + activeWeapon->getSpread());
    for (int i = 0; i < 100 && hitChance < minChance; i++) {
        QAngle randomSpreadAngle = {randFloat(0, spread) - (spread / 2), randFloat(0, spread) - (spread / 2),
                                    randFloat(0, spread) - (spread / 2)};

        angleVectors(viewAngles + randomSpreadAngle, endPos);

        endPos = EntityCache::localPlayer->eyepos() + (endPos *= 4096.f);

        Entity* e = clipRayToEntity(EntityCache::localPlayer->eyepos(), endPos, &traceToPlayer, data.enter_trace.m_pEntityHit);
        if (!e || e == EntityCache::localPlayer || e->teammate() || e->nDT_BasePlayer__m_iHealth() == 0 ||
            e->nDT_CSPlayer__m_bGunGameImmunity())
            continue;

        hitChance++;
    }

    return hitChance >= minChance;
}