#include "lua.hpp"
#include "luabridge/LuaBridge.h"
#include "../util/log.hpp"
#include "../menu/config.hpp"
#include "../menu/menu.hpp"

namespace Lua {
    LuaEngine* curEngineBeingRan; // used for registerHook to know the engine the func is being ran from

    namespace Cheat {
        void registerHook(const char* hook, const char* funcName) {
            curEngineBeingRan->hooks.insert(std::pair<std::string, std::string>(hook, funcName));
        }
    }
    namespace UI {
        ImVec2 getMenuPos() {
            return ImGui::GetWindowPos();
        }
        ImVec2 getMenuSize() {
            return ImGui::GetWindowSize();
        }
        bool isMenuOpen() {
            return Menu::menuOpen;
        }
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
        void separator() {
            ImGui::Separator();
        }
        void label(const char* label) {
            ImGui::TextWrapped("%s", label);
        }
        bool checkbox(const char* label, const char* configVarName) {
            return ImGui::Checkbox(label, &CONFIGBOOL(configVarName));
        }
        bool button(const char* label) {
            return ImGui::Button(label, ImVec2(ImGui::GetWindowContentRegionWidth(), 16));
        }
        void textInput(const char* label, const char* configVarName) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            char buf[1024];
            strcpy(buf, CONFIGSTR(configVarName).c_str());
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
            ImGui::InputText("##pog", buf, sizeof(buf));
            CONFIGSTR(configVarName) = buf;
        }
        void textInputMultiline(const char* label, const char* configVarName) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            char buf[1024];
            strcpy(buf, CONFIGSTR(configVarName).c_str());
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
            ImGui::InputTextMultiline("##pog", buf, sizeof(buf));
            CONFIGSTR(configVarName) = buf;
        }
    }
    namespace Draw {
        void rectangle(ImVec2 min, ImVec2 max, ImColor color, float thickness)                          { curDrawList->AddRect(min, max, color, 0, 0, thickness); }
        void rectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float thickness, float rounding)   { curDrawList->AddRect(min, max, color, rounding, 0, thickness); }
        void filledRectangle(ImVec2 min, ImVec2 max, ImColor color)                                     { curDrawList->AddRectFilled(min, max, color); }
        void filledRectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float rounding)              { curDrawList->AddRectFilled(min, max, color, rounding); }
        
        void circle(ImVec2 center, float radius, ImColor color, float thickness)                        { curDrawList->AddCircle(center, radius, color, 0, thickness); }
        void filledCircle(ImVec2 center, float radius, ImColor color)                                   { curDrawList->AddCircleFilled(center, radius, color); }

        void text(ImVec2 pos, ImColor color, const char* text)                                          { curDrawList->AddText(pos, color, text); }
        void shadowText(ImVec2 pos, ImColor color, const char* text) {
            curDrawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0), text);
            curDrawList->AddText(pos, color, text);
        }
        void outlineText(ImVec2 pos, ImColor color, const char* text) {
            curDrawList->AddText(ImVec2(pos.x - 1, pos.y), ImColor(0, 0, 0), text);
            curDrawList->AddText(ImVec2(pos.x + 1, pos.y), ImColor(0, 0, 0), text);
            curDrawList->AddText(ImVec2(pos.x, pos.y - 1), ImColor(0, 0, 0), text);
            curDrawList->AddText(ImVec2(pos.x, pos.y + 1), ImColor(0, 0, 0), text);
            curDrawList->AddText(pos, color, text);
        }

        ImVec2 getScreenSize() {
            return ImGui::GetIO().DisplaySize;
        }

        float deltaTime() {
            return ImGui::GetIO().DeltaTime;
        }
    }    

    void bridge(lua_State* L) {
        luabridge::getGlobalNamespace(L)
            .beginClass<ImVec2>("vec2")
                .addConstructor<void (*) (float, float)>()
                .addProperty("x", &ImVec2::x)
                .addProperty("y", &ImVec2::y)
            .endClass()
            .beginClass<ImVec4>("vec4")
                .addConstructor<void (*) (float, float, float, float)>()
                .addProperty("x", &ImVec4::x)
                .addProperty("y", &ImVec4::y)
                .addProperty("z", &ImVec4::z)
                .addProperty("w", &ImVec4::w)
            .endClass()
            .beginClass<ImColor>("color")
                .addConstructor<void (*) (float, float, float, float)>()
            .endClass()
            .beginNamespace("cheat")
                .addFunction("registerHook", Cheat::registerHook)
            .endNamespace()
            .beginNamespace("ui")
                .addFunction("getMenuPos", UI::getMenuPos)
                .addFunction("getMenuSize", UI::getMenuSize)
                .addFunction("isMenuOpen", UI::isMenuOpen)
                .addFunction("getConfigBool", UI::getConfigBool)
                .addFunction("setConfigBool", UI::setConfigBool)
                .addFunction("getConfigFloat", UI::getConfigFloat)
                .addFunction("setConfigFloat", UI::setConfigFloat)
                .addFunction("getConfigInt", UI::getConfigInt)
                .addFunction("setConfigInt", UI::setConfigInt)
                .addFunction("getConfigStr", UI::getConfigStr)
                .addFunction("setConfigStr", UI::setConfigStr)
                .addFunction("separator", UI::separator)
                .addFunction("label", UI::label)
                .addFunction("checkbox", UI::checkbox)
                .addFunction("button", UI::button)
                .addFunction("textInput", UI::textInput)
                .addFunction("textInputMultiline", UI::textInputMultiline)
            .endNamespace()
            .beginNamespace("draw")
                .addFunction("rectangle", Draw::rectangle)
                .addFunction("rectangleRounded", Draw::rectangleRounded)
                .addFunction("filledRectangle", Draw::filledRectangle)
                .addFunction("filledRectangleRounded", Draw::filledRectangleRounded)

                .addFunction("circle", Draw::circle)
                .addFunction("filledCircle", Draw::filledCircle)

                .addFunction("text", Draw::text)
                .addFunction("shadowText", Draw::shadowText)
                .addFunction("outlineText", Draw::outlineText)

                .addFunction("getScreenSize", Draw::getScreenSize)
                .addFunction("deltaTime", Draw::deltaTime)
                .addFunction("HSVtoColor", ImColor::HSV)
            .endNamespace();
    }

    void handleHook(const char* hook) {
        for (auto& engine : scripts) {
            if (engine.second.hooks.find(hook) != engine.second.hooks.end()) {
                luabridge::LuaRef funcRef = luabridge::getGlobal(engine.second.state, engine.second.hooks.at(hook).c_str());
                if (strstr(hook, "UI")) {
                    if (ImGui::CollapsingHeader(engine.first.c_str())) {
                        try {
                            funcRef();
                        }
                        catch (luabridge::LuaException const& e) {
                            ERR("lua error: %s", e.what());
                        }
                    }
                    ImGui::Separator();
                }
                else {
                    try {
                        funcRef();
                    }
                    catch (luabridge::LuaException const& e) {
                        ERR("lua error: %s", e.what());
                    }
                }
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
        if (luaL_dofile(state, path)) {
            ERR("lua error: %s", lua_tostring(state, -1));
        }
    }
}