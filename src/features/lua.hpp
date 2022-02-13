#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <map>
#include <mutex>

#include <luajit-2.0/lua.hpp>
#include <luajit-2.0/lualib.h>

#include "luabridge/LuaBridge.h"
#include "luabridge/detail/LuaRef.h"
#include "luabridge/Vector.h"

#include "../menu/imgui/imgui.h"
#include "../interfaces.hpp"

namespace Lua {
    class LuaGameEvent {
        public:
        IGameEvent* e;
        uintptr_t ffiPtr();
        bool getBool(const char* name);
        float getFloat(const char* name);
        int getInt(const char* name);
        const char* getName();
        uintptr_t getPtr(const char* name);
        const char* getString(const char* name);
        uint64_t getUint64(const char* name);
        const wchar_t* getWString(const char* name);
        LuaGameEvent(IGameEvent* event);
    };

    class LuaEngine {
        public:
        lua_State* state;
        std::map<std::string, luabridge::LuaRef> hooks;
        LuaEngine(std::string filename);
    };

    inline std::map<std::string, LuaEngine> scripts;
    inline std::vector<std::string> luaFiles;

    inline LuaEngine* curEngineBeingRan; // used for registerHook to know the engine the func is being ran from
    inline const char* curEngineBeingRanName; // used for registerHook to know the engine the func is being ran from

    inline ImDrawList* curDrawList;
    inline CUserCmd* curCmd;

    inline std::mutex luaLock;

    void handleHook(const char* hook);

    inline void refreshLuaList() {
        luaFiles.clear();
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.eclipse/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file())
                luaFiles.push_back(entry.path().string().substr(strlen(path)));
        }
        std::sort(luaFiles.begin(), luaFiles.end());
    }

    template <class T1>
    inline void handleHook(const char* hook, T1 arg1) {
        std::lock_guard<std::mutex> lock(luaLock);
        for (auto& engine : scripts) {
            curEngineBeingRan = &engine.second;
            curEngineBeingRanName = engine.first.c_str();
            if (engine.second.hooks.find(hook) != engine.second.hooks.end()) {
                INFO("running %s for %s", hook, engine.first.c_str());
                try { 
                    engine.second.hooks.at(hook)(arg1);
                }
                catch (luabridge::LuaException const& e) {
                    ERR("lua error (%s) (%s): %s", engine.first.c_str(), hook, e.what());
                }
                INFO("ran %s for %s", hook, engine.first.c_str());
            }
        }
    }
}