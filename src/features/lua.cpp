#include "lua.hpp"
#include "luabridge/LuaBridge.h"
#include "luabridge/detail/LuaRef.h"
#include "luabridge/Vector.h"
#include "../interfaces.hpp"
#include "../util/log.hpp"
#include "../menu/config.hpp"
#include "../menu/menu.hpp"
#include "../menu/imgui/imgui_internal.h"
#include "../sdk/entity.hpp"
#include "../sdk/netvars.hpp"

namespace Lua {
    LuaEngine* curEngineBeingRan; // used for registerHook to know the engine the func is being ran from

    class LuaEntity {
        public:
        Entity* e;
        uintptr_t ffiPtr() {return (uintptr_t)e;}
        bool exists() {return e;}
        bool dormant() {return e->dormant();}
        bool sane() {return e && !e->dormant();}
        int index() {return e->index();}
        
        Vector origin() {return e->origin();}

        template <typename T>
        T getProp(const char* table, const char* varName) {
            return *reinterpret_cast<T*>((uintptr_t)e + Netvars::netvars[std::pair<std::string_view, std::string_view>(table, varName)].second);
        }

        template <typename T>
        void setProp(const char* table, const char* varName, T value) {
            *reinterpret_cast<T*>((uintptr_t)e + Netvars::netvars[std::pair<std::string_view, std::string_view>(table, varName)].second) = value;
        }

        LuaEntity(Entity* ent) {
            e = ent;
        }
    };

    namespace Cheat {
        void registerHook(const char* hook, const char* funcName) {
            curEngineBeingRan->hooks.insert(std::pair<std::string, std::string>(hook, funcName));
        }

        uintptr_t getInterface(const char* file, const char* name) { return (uintptr_t)Interfaces::getInterface<void*>(file, name); }

        CUserCmd getCmd() { return *curCmd; }
        void setCmd(CUserCmd cmd) { memcpy((void*)curCmd, (void*)&cmd, sizeof(CUserCmd)); }

        LuaEntity getEntity(int i) {
            Entity* ent = (Entity*)Interfaces::entityList->getClientEntity(i);
            if (ent) {
                return LuaEntity(ent);
            }
        }

        std::vector<LuaEntity> getEntities() {
            std::vector<LuaEntity> entities;
            for (int i = 0; i <= Interfaces::entityList->getHighestEntityIndex(); i++) {
                Entity* ent = (Entity*)Interfaces::entityList->getClientEntity(i);
                if (ent) {
                    entities.push_back(LuaEntity(ent));
                }
            }
            return entities;
        }

        std::vector<LuaEntity> getEntitiesByClassID(int classID) {
            std::vector<LuaEntity> entities;
            for (int i = 0; i <= Interfaces::entityList->getHighestEntityIndex(); i++) {
                Entity* ent = (Entity*)Interfaces::entityList->getClientEntity(i);
                if (ent && ent->clientClass()->m_ClassID == classID) {
                    entities.push_back(LuaEntity(ent));
                }
            }
            return entities;
        }
    }
    namespace UI {
        ImVec2 getMenuPos() { return Menu::windowPos; }
        ImVec2 getMenuSize() { return Menu::windowSize; }
        bool isMenuOpen() { return Menu::menuOpen; }
        bool getConfigBool(const char* var) { return CONFIGBOOL(var); }
        void setConfigBool(const char* var, bool val) { CONFIGBOOL(var) = val; }
        float getConfigFloat(const char* var) { return CONFIGFLOAT(var); }
        void setConfigFloat(const char* var, float val) { CONFIGFLOAT(var) = val; }
        int getConfigInt(const char* var) { return CONFIGINT(var); }
        void setConfigInt(const char* var, int val) { CONFIGINT(var) = val; }
        const char* getConfigStr(const char* var) { return CONFIGSTR(var).c_str(); }
        void setConfigStr(const char* var, const char* val) { CONFIGSTR(var) = val; }
        ImColor getConfigCol(const char* var) { return CONFIGCOL(var); }
        void setConfigCol(const char* var, ImColor val) { CONFIGCOL(var) = val; }
        void beginWindow(const char* title) { ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus); }
        void endWindow() { ImGui::End(); }
        void sameLine() { ImGui::SameLine(); }
        void columns(int count, bool border) { ImGui::Columns(count, nullptr, border); }
        void nextColumn() { ImGui::NextColumn(); }
        void separator() { ImGui::Separator(); }
        void label(const char* label) { ImGui::TextWrapped("%s", label); }
        bool checkbox(const char* label, const char* configVarName) {
            ImGui::Checkbox(label, &CONFIGBOOL(configVarName));
            return CONFIGBOOL(configVarName);
        }
        bool button(const char* label) {
            return ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvailWidth(), 16));
        }
        ImColor colorPicker(const char* label, const char* configVarName) {
            ImGui::ColorEdit4(label, (float*)&CONFIGCOL(configVarName).Value, ImGuiColorEditFlags_NoInputs);
            return CONFIGCOL(configVarName);
        }
        const char* textInput(const char* label, const char* configVarName) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            char buf[1024];
            strcpy(buf, CONFIGSTR(configVarName).c_str());
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::InputText(labelBuf, buf, sizeof(buf));
            CONFIGSTR(configVarName) = buf;
            return CONFIGSTR(configVarName).c_str();
        }
        const char* textInputMultiline(const char* label, const char* configVarName) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            char buf[1024];
            strcpy(buf, CONFIGSTR(configVarName).c_str());
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::InputTextMultiline(labelBuf, buf, sizeof(buf));
            CONFIGSTR(configVarName) = buf;
            return CONFIGSTR(configVarName).c_str();
        }
        int sliderInt(const char* label, const char* configVarName, int min, int max, const char* format) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::SliderInt(labelBuf, &CONFIGINT(configVarName), min, max, format);
            return CONFIGINT(configVarName);
        }
        float sliderFloat(const char* label, const char* configVarName, float min, float max, const char* format) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::SliderFloat(labelBuf, &CONFIGFLOAT(configVarName), min, max, format);
            return CONFIGFLOAT(configVarName);
        }
    }
    namespace Draw {
        void rectangle(ImVec2 min, ImVec2 max, ImColor color, float thickness)                          { curDrawList->AddRect(min, max, color, 0, 0, thickness); }
        void rectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float thickness, float rounding)   { curDrawList->AddRect(min, max, color, rounding, 0, thickness); }
        void filledRectangle(ImVec2 min, ImVec2 max, ImColor color)                                     { curDrawList->AddRectFilled(min, max, color); }
        void filledRectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float rounding)              { curDrawList->AddRectFilled(min, max, color, rounding); }
        void gradientFilledRectangle(ImVec2 min, ImVec2 max, ImColor tl, ImColor tr, ImColor bl, ImColor br) { curDrawList->AddRectFilledMultiColor(min, max, tl, tr, br, bl); }
        
        void circle(ImVec2 center, float radius, ImColor color, float thickness)                        { curDrawList->AddCircle(center, radius, color, 0, thickness); }
        void filledCircle(ImVec2 center, float radius, ImColor color)                                   { curDrawList->AddCircleFilled(center, radius, color); }

        ImVec2 calcTextSize(const char* text)                                                             { return ImGui::CalcTextSize(text); }
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
            .beginClass<ImVec2>("Vec2")
                .addConstructor<void (*) (float, float)>()
                .addProperty("x", &ImVec2::x)
                .addProperty("y", &ImVec2::y)
            .endClass()
            .beginClass<ImVec4>("Vec4")
                .addConstructor<void (*) (float, float, float, float)>()
                .addProperty("x", &ImVec4::x)
                .addProperty("y", &ImVec4::y)
                .addProperty("z", &ImVec4::z)
                .addProperty("w", &ImVec4::w)
            .endClass()
            .beginClass<ImColor>("Color")
                .addConstructor<void (*) (float, float, float, float)>()
                .addProperty("value", &ImColor::Value)
            .endClass()
            .beginClass<Vector>("Vector")
                .addConstructor<void (*) (float, float, float)>()
                .addProperty("x", &Vector::x)
                .addProperty("y", &Vector::y)
                .addProperty("z", &Vector::z)
                .addFunction("length", &Vector::Length)
                .addFunction("length2D", &Vector::Length2D)
            .endClass()
            .beginClass<QAngle>("QAngle")
                .addConstructor<void (*) (float, float, float)>()
                .addProperty("x", &QAngle::x)
                .addProperty("y", &QAngle::y)
                .addProperty("z", &QAngle::z)
                .addFunction("length", &QAngle::Length)
            .endClass()
            .beginClass<CUserCmd>("userCmd")
                .addProperty("commandnumber", &CUserCmd::commandnumber)
                .addProperty("tickcount", &CUserCmd::tickcount)
                .addProperty("viewangles", &CUserCmd::viewangles)
                .addProperty("aimdirection", &CUserCmd::aimdirection)
                .addProperty("forwardmove", &CUserCmd::forwardmove)
                .addProperty("sidemove", &CUserCmd::sidemove)
                .addProperty("upmove", &CUserCmd::upmove)
                .addProperty("buttons", &CUserCmd::buttons)
                .addProperty("impulse", &CUserCmd::impulse)
                .addProperty("weaponselect", &CUserCmd::weaponselect)
                .addProperty("upmove", &CUserCmd::randomseed)
                .addProperty("mousedx", &CUserCmd::mousedx)
                .addProperty("mousedy", &CUserCmd::mousedy)
                .addProperty("hasbeenpredicted", &CUserCmd::hasbeenpredicted)
                .addProperty("headangles", &CUserCmd::headangles)
                .addProperty("headoffset", &CUserCmd::headoffset)
            .endClass()
            .beginClass<LuaEntity>("Entity")
                .addFunction("ffiPtr", &LuaEntity::ffiPtr)
                .addFunction("exists", &LuaEntity::exists)
                .addFunction("dormant", &LuaEntity::dormant)
                .addFunction("sane", &LuaEntity::exists)
                .addFunction("index", &LuaEntity::index)
                .addFunction("origin", &LuaEntity::origin)
                .addFunction("getPropBool", &LuaEntity::getProp<bool>)
                .addFunction("getPropInt", &LuaEntity::getProp<int>)
                .addFunction("getPropFloat", &LuaEntity::getProp<float>)
                .addFunction("getPropFloat", &LuaEntity::getProp<float>)
                .addFunction("getPropPtr", &LuaEntity::getProp<unsigned int>)
                .addFunction("getPropQAngle", &LuaEntity::getProp<QAngle>)
                .addFunction("getPropVector", &LuaEntity::getProp<Vector>)
                .addFunction("setPropBool", &LuaEntity::setProp<bool>)
                .addFunction("setPropInt", &LuaEntity::setProp<int>)
                .addFunction("setPropFloat", &LuaEntity::setProp<float>)
                .addFunction("setPropFloat", &LuaEntity::setProp<float>)
                .addFunction("setPropPtr", &LuaEntity::setProp<unsigned int>)
                .addFunction("setPropQAngle", &LuaEntity::setProp<QAngle>)
                .addFunction("setPropVector", &LuaEntity::setProp<Vector>)
            .endClass()
            .beginNamespace("cheat")
                .addFunction("registerHook", Cheat::registerHook)
                .addFunction("getInterface", Cheat::getInterface)
                .addFunction("getCmd", Cheat::getCmd)
                .addFunction("setCmd", Cheat::setCmd)
                .addFunction("getEntity", Cheat::getEntity)
                .addFunction("getEntities", Cheat::getEntities)
                .addFunction("getEntitiesByClassID", Cheat::getEntitiesByClassID)
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
                .addFunction("getConfigCol", UI::getConfigCol)
                .addFunction("setConfigCol", UI::setConfigCol)
                .addFunction("beginWindow", UI::beginWindow)
                .addFunction("endWindow", UI::endWindow)
                .addFunction("sameLine", UI::sameLine)
                .addFunction("columns", UI::columns)
                .addFunction("nextColumn", UI::nextColumn)
                .addFunction("separator", UI::separator)
                .addFunction("label", UI::label)
                .addFunction("checkbox", UI::checkbox)
                .addFunction("button", UI::button)
                .addFunction("colorPicker", UI::colorPicker)
                .addFunction("textInput", UI::textInput)
                .addFunction("textInputMultiline", UI::textInputMultiline)
                .addFunction("sliderInt", UI::sliderInt)
                .addFunction("sliderFloat", UI::sliderFloat)
            .endNamespace()
            .beginNamespace("draw")
                .addFunction("rectangle", Draw::rectangle)
                .addFunction("rectangleRounded", Draw::rectangleRounded)
                .addFunction("filledRectangle", Draw::filledRectangle)
                .addFunction("filledRectangleRounded", Draw::filledRectangleRounded)
                .addFunction("gradientFilledRectangle", Draw::gradientFilledRectangle)

                .addFunction("circle", Draw::circle)
                .addFunction("filledCircle", Draw::filledCircle)

                .addFunction("calcTextSize", Draw::calcTextSize)
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
                int oldStackSize = 0;
                if (strstr(hook, "draw"))
                    oldStackSize = ImGui::GetCurrentContext()->CurrentWindowStack.Size;

                if (strstr(hook, "UI")) {
                    if (ImGui::CollapsingHeader(engine.first.c_str())) {
                        try {
                            funcRef();
                        }
                        catch (luabridge::LuaException const& e) {
                            ERR("lua error (%s): %s", engine.first.c_str(), e.what());
                        }
                    }
                    ImGui::Separator();
                }
                else {
                    try { 
                        funcRef();
                    }
                    catch (luabridge::LuaException const& e) {
                        ERR("lua error (%s): %s", engine.first.c_str(), e.what());
                    }
                }
                if (strstr(hook, "draw")) {
                    while (ImGui::GetCurrentContext()->CurrentWindowStack.Size > oldStackSize) {
                        ImGui::End();
                        WARN("lua %s: ui.beginWindow missing ui.endWindow", engine.first.c_str());
                    }
                }
            }
        }
    }

    LuaEngine::LuaEngine(std::string filename) {
        char path[512], filepath[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        strcpy(filepath, path);
        strcat(filepath, filename.c_str());

        state = luaL_newstate();
        luaL_openlibs(state);
        
        bridge(state); // add c++ funcs to lua
        
        luaL_dostring(state, (std::string("package.path = \"") + std::string(path) + std::string("?.lua;\" .. package.path")).c_str());

        curEngineBeingRan = this;
        if (luaL_dofile(state, filepath)) {
            ERR("lua error: %s", lua_tostring(state, -1));
        }
    }
}