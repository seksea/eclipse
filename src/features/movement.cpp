#include "movement.hpp"
#include "../sdk/entity.hpp"
#include "../menu/config.hpp"

namespace Movement {
    void bunnyhop(CUserCmd* cmd) {
        if (!CONFIGBOOL("bhop"))
            return;
        if (EntityCache::localPlayer->moveType() == 9)
            return;

        if (!(EntityCache::localPlayer->nDT_BasePlayer__m_fFlags() & FL_ONGROUND))
            cmd->buttons &= ~IN_JUMP;
    }
}