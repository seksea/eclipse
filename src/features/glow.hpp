#pragma once
#include <vector>

namespace Glow {
    inline const char* styles[] = {"classic", "rim", "stencil", "stencil pulse"};
    inline std::vector<std::pair<int, int>> customGlowEntities;
    void draw();
}