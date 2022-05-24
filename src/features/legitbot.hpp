#pragma once
#include "../interfaces.hpp"

namespace Legitbot {
    enum class HitBoxes {
        HEAD = 1 << 0,
        NECK = 1 << 1,
        CHEST = 1 << 2,
        STOMACH = 1 << 3,
        PELVIS = 1 << 4,
    };
    bool shouldHit(QAngle viewAngles, int minChance);
    void aimbot(CUserCmd* cmd);
    void triggerbot(CUserCmd* cmd);
    void run(CUserCmd* cmd);
}