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
#include "../menu/imgui/imgui_internal.h"
#include "../interfaces.hpp"

namespace Lua {
    class LuaEngine {
        public:
        lua_State* state;
        std::map<std::string, luabridge::LuaRef> hooks;
        LuaEngine(std::string filename);
    };

    inline LuaEngine* curEngineBeingRan; // used for registerHook to know the engine the func is being ran from
    inline const char* curEngineBeingRanName; // used for registerHook to know the engine the func is being ran from

    inline ImDrawList* curDrawList;
    inline CUserCmd* curCmd;

    inline std::mutex luaLock;

    inline std::map<std::string, LuaEngine> scripts;
    inline std::vector<std::string> luaFiles;

    void bridge(lua_State* L);
    
    template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
    inline void handleHook(const char* hookName, P1 const& arg1 = 0, P2 const& arg2 = 0, P3 const& arg3 = 0, 
                        P4 const& arg4 = 0, P5 const& arg5 = 0, P6 const& arg6 = 0, P7 const& arg7 = 0, P8 const& arg8 = 0) {
        std::lock_guard<std::mutex> lock(luaLock);
        for (auto& engine : scripts) {
            curEngineBeingRan = &engine.second;
            curEngineBeingRanName = engine.first.c_str();

            if (engine.second.hooks.find(hookName) != engine.second.hooks.end()) {
                int oldStackSize = 0;
                if (strstr(hookName, "draw")) {
                    oldStackSize = ImGui::GetCurrentContext()->CurrentWindowStack.Size;
                }
                
                if (strstr(hookName, "UI")) {
                    if (ImGui::CollapsingHeader(engine.first.c_str())) {
                        try {
                            engine.second.hooks.at(hookName)();
                        }
                        catch (luabridge::LuaException const& e) {
                            ERR("lua error (%s) (%s): %s", engine.first.c_str(), hookName, e.what());
                        }
                    }
                    ImGui::Separator();
                }
                else {
                    try {
                        engine.second.hooks.at(hookName).push();
                        luabridge::Stack<P1>::push(engine.second.state, arg1);
                        luabridge::Stack<P2>::push(engine.second.state, arg2);
                        luabridge::Stack<P3>::push(engine.second.state, arg3);
                        luabridge::Stack<P4>::push(engine.second.state, arg4);
                        luabridge::Stack<P5>::push(engine.second.state, arg5);
                        luabridge::Stack<P6>::push(engine.second.state, arg6);
                        luabridge::Stack<P7>::push(engine.second.state, arg7);
                        luabridge::Stack<P8>::push(engine.second.state, arg8);
                        luabridge::LuaException::pcall(engine.second.state, 8, 1);
                    }
                    catch (luabridge::LuaException const& e) {
                        ERR("lua error (%s) (%s): %s", engine.first.c_str(), hookName, e.what());
                    }
                }

                if (strstr(hookName, "draw")) {
                    while (ImGui::GetCurrentContext()->CurrentWindowStack.Size > oldStackSize) {
                        ImGui::End();
                        WARN("lua %s: ui.beginWindow missing ui.endWindow", engine.first.c_str());
                    }
                }
            }
        }
    }

    inline void refreshLuaList() {
        luaFiles.clear();
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.eclipse/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            luaFiles.push_back(entry.path().string().substr(strlen(path)));
        }
        std::sort(luaFiles.begin(), luaFiles.end());
    }
}