#pragma once
#include <vector>
#include <map>

#include "../interfaces.hpp"

namespace Backtrack {
    struct Player {
        matrix3x4_t boneMatrix[128];
        float simTime;
    };

    struct Tick {
        std::map<int, Player> players;
        int tickCount;
    };

    inline std::vector<Tick> ticks;

    void store(CUserCmd* cmd);
    void run(CUserCmd* cmd);
}