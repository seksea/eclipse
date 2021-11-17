#include "lua.hpp"
#include "luabridge/LuaBridge.h"
#include "../util/log.hpp"
#include "../menu/imgui/imgui.h"
#include "../menu/config.hpp"

namespace Lua {
    LuaEngine* curEngineBeingRan; // used for registerHook to know the engine the func is being ran from

    namespace Cheat {
        void registerHook(const char* hook, const char* funcName) {
            curEngineBeingRan->hooks.insert(std::pair<std::string, std::string>(hook, funcName));
        }
    }
    namespace UI {
        bool getConfigBool(const char* var) {
            return CONFIGBOOL(var);
        }
        void setConfigBool(const char* var, bool val) {
            CONFIGBOOL(var) = val;
        }
        float getConfigFloat(const char* var) {
            return CONFIGFLOAT(var);
        }
        void setConfigFloat(const char* var, float val) {
            CONFIGFLOAT(var) = val;
        }
        int getConfigInt(const char* var) {
            return CONFIGINT(var);
        }
        void setConfigInt(const char* var, int val) {
            CONFIGINT(var) = val;
        }
        const char* getConfigStr(const char* var) {
            return CONFIGSTR(var).c_str();
        }
        void setConfigStr(const char* var, const char* val) {
            CONFIGSTR(var) = val;
        }
        void label(const char* label) {
            ImGui::TextWrapped("%s", label);
        }
        bool checkbox(const char* label, const char* configVarName) {
            return ImGui::Checkbox(label, &CONFIGBOOL(configVarName));
        }
    }
    namespace Draw {
        void filledRectangle(int x, int y, int x2, int y2, unsigned int color) {
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x2, y2), ImColor(color));
        }
    }

    

    void bridge(lua_State* L) {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("cheat")
                .addFunction("registerHook", Cheat::registerHook)
            .endNamespace()
            .beginNamespace("ui")
                .addFunction("getConfigBool", UI::getConfigBool)
                .addFunction("setConfigBool", UI::setConfigBool)
                .addFunction("getConfigFloat", UI::getConfigFloat)
                .addFunction("setConfigFloat", UI::setConfigFloat)
                .addFunction("getConfigInt", UI::getConfigInt)
                .addFunction("setConfigInt", UI::setConfigInt)
                .addFunction("getConfigStr", UI::getConfigStr)
                .addFunction("setConfigStr", UI::setConfigStr)
                .addFunction("label", UI::label)
                .addFunction("checkbox", UI::checkbox)
            .endNamespace()
            .beginNamespace("draw")
                .addFunction("filledRectangle", Draw::filledRectangle)
            .endNamespace();
    }

    void handleHook(const char* hook) {
        for (auto& engine : scripts) {
            if (engine.second.hooks.find(hook) != engine.second.hooks.end()) {
                if (strstr(hook, "UI")) {
                    ImGui::Text("%s", engine.first.c_str());
                    ImGui::Separator();
                }
                luabridge::LuaRef funcRef = luabridge::getGlobal(engine.second.state, engine.second.hooks.at(hook).c_str());
                funcRef();
            }
        }
    }

    LuaEngine::LuaEngine(std::string filename) {
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        strcat(path, filename.c_str());

        state = luaL_newstate();
        luaL_openlibs(state);
        
        bridge(state); // add c++ funcs to lua
        
        curEngineBeingRan = this;
        luaL_dofile(state, path);
    }
}