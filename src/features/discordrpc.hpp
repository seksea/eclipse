#pragma once
#include "gamesdk/discord.h"

namespace DiscordRPC {
    inline discord::Core* core{};
    void tick();
}