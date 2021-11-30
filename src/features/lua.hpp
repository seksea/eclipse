#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <map>

#include <luajit-2.0/lua.hpp>
#include <luajit-2.0/lualib.h>

#include "../menu/imgui/imgui.h"
#include "../interfaces.hpp"

namespace Lua {
    inline ImDrawList* curDrawList;
    inline CUserCmd* curCmd;

    class LuaEngine {
        public:
        lua_State* state;
        std::map<std::string, std::string> hooks;
        LuaEngine(std::string filename);
    };

    inline std::map<std::string, LuaEngine> scripts;
    inline std::vector<std::string> luaFiles;

    void bridge(lua_State* L);
    void handleHook(const char* hook);

    inline void refreshLuaList() {
        luaFiles.clear();
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            luaFiles.push_back(entry.path().string().substr(strlen(path)));
        }
        std::sort(luaFiles.begin(), luaFiles.end());
    }
}