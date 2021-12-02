#include "discordrpc.hpp"
#include "../interfaces.hpp"
#include <chrono>

namespace DiscordRPC {
    void tick() {
        static bool init = false;
        if (!init) {
            core->Create(880767080671117342, DiscordCreateFlags_Default, &core);
            init = true;
        }
        discord::Activity activity{};
        activity.SetState("");
        activity.SetDetails(Interfaces::engine->isInGame() ? Interfaces::engine->getLevelName() : "in the lobby");
        activity.GetAssets().SetLargeImage("eclipselogo");
        activity.GetAssets().SetLargeText("eclipse.wtf");
        activity.GetAssets().SetSmallImage(Interfaces::engine->isInGame() ? Interfaces::engine->getLevelName() : "");
        activity.GetAssets().SetSmallText(Interfaces::engine->isInGame() ? Interfaces::engine->getLevelName() : "");
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            if (result != discord::Result::Ok) {
                WARN("Failed to update discord game sdk activity: %i", result);
            }
        });
        core->RunCallbacks();
    }
}