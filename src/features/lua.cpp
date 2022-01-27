#include "lua.hpp"
#include "../interfaces.hpp"
#include "../hooks.hpp"
#include "../util/log.hpp"
#include "../menu/config.hpp"
#include "../menu/menu.hpp"
#include "../menu/imgui/imgui_internal.h"
#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"
#include "../sdk/netvars.hpp"
#include "chams.hpp"

namespace Lua {
    void print(const char* str) {
        LOG("[%s] %s", curEngineBeingRanName, str);
    }

    class LuaEntity {
        public:
        Entity* e;
        uintptr_t ffiPtr() {return (uintptr_t)e;}
        bool exists() {return e;}
        bool dormant() {return e->dormant();}
        bool sane() {return e && !e->dormant();}
        bool alive() {return e->nDT_BasePlayer__m_iHealth() > 0;}
        bool onground() {return e->nDT_BasePlayer__m_fFlags() & FL_ONGROUND;}
        int movetype() {return e->moveType();}
        int index() {return e->index();}
        int classID() {return e->clientClass()->m_ClassID;}
        const char* networkName() {return e->clientClass()->m_pNetworkName;}
        ImVec4 getBBox() {return getBoundingBox(e);}
        void setMins(Vector mins) { e->nDT_BaseEntity__m_Collision().OBBMins() = mins; }
        void setMaxs(Vector maxs) { e->nDT_BaseEntity__m_Collision().OBBMaxs() = maxs; }
        void onPreDataChanged(int updateType) { e->onPreDataChanged(updateType); }
        void onDataChanged(int updateType) { e->onDataChanged(updateType); }
        void preDataUpdate(int updateType) { e->preDataUpdate(updateType); }
        void postDataUpdate(int updateType) { e->postDataUpdate(updateType); }

        Vector origin() {return e->origin();}
        Vector velocity() {return Vector(e->nDT_LocalPlayerExclusive__m_vecVelocity0(), e->nDT_LocalPlayerExclusive__m_vecVelocity1(), e->nDT_LocalPlayerExclusive__m_vecVelocity2()); }

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

    class LuaClientClass {
        public:
        ClientClass* c;
        bool exists() {return c;}
        LuaEntity create(int entNum, int serialNum) {
            return LuaEntity((Entity*)c->m_pCreateFn(entNum, serialNum));
        }
	    const char* name() {
            return c->m_pNetworkName;
        };
	    LuaClientClass next() {
            return LuaClientClass(c->m_pNext);
        };
        LuaClientClass(ClientClass* _c) {
            this->c = _c;
        }
    };

    class LuaConvar {
        public:
        Convar* c;
        uintptr_t ffiPtr() {return (uintptr_t)c;}
        float getFloat() {return c->getFloat();}
        int getInt() {return c->getInt();}
        void setString(const char* val) {c->setString(val);}
        void setFloat(float val) {c->setFloat(val);}
        void setInt(int val) {c->setInt(val);}
        LuaConvar(Convar* convar) {
            c = convar;
        }
    };

    uintptr_t LuaGameEvent::ffiPtr() {return (uintptr_t)e;}
    bool LuaGameEvent::getBool(const char* name) { return e->getBool(name); }
    float LuaGameEvent::getFloat(const char* name) { return e->getFloat(name); }
    int LuaGameEvent::getInt(const char* name) { return e->getInt(name); }
    const char* LuaGameEvent::getName() { return e->getName(); }
    uintptr_t LuaGameEvent::getPtr(const char* name) { return (uintptr_t)e->getPtr(name); }
    const char* LuaGameEvent::getString(const char* name) { return e->getString(name); }
    uint64_t LuaGameEvent::getUint64(const char* name) { return e->getUint64(name); }
    const wchar_t* LuaGameEvent::getWString(const char* name) { return e->getWString(name); }
    LuaGameEvent::LuaGameEvent(IGameEvent* event) {
        e = event;
    }

    namespace Eclipse {
        void registerHook(const char* hook, luabridge::LuaRef func) { curEngineBeingRan->hooks.insert(std::pair<std::string, luabridge::LuaRef>(hook, func)); }

        void setCmd(CUserCmd cmd) { memcpy((void*)curCmd, (void*)&cmd, sizeof(CUserCmd)); }

        LuaConvar getConvar(const char* name) { return LuaConvar(Interfaces::cvar->findVar(name)); }

        ImVec2 _worldToScreen(Vector pos) { Vector screenPos; worldToScreen(pos, screenPos); return ImVec2(screenPos.x, screenPos.y); }
        
        LuaClientClass getAllClientClasses() { return LuaClientClass(Interfaces::client->getAllClasses()); }

        void addEventListener(const char* eventName, bool serverSide) { Interfaces::eventManager->addListener(Hooks::eventListener, eventName, serverSide); }

        void clientCmd(const char* command) { return Interfaces::engine->clientCmdUnrestricted(command); }

        bool isInGame() { return Interfaces::engine->isInGame(); }

        CUserCmd startMovementFix_(CUserCmd cmd) { startMovementFix(&cmd); return cmd; }

        CUserCmd endMovementFix_(CUserCmd cmd) { endMovementFix(&cmd); return cmd; }
    }

    namespace Mem {
        uintptr_t getInterface(const char* file, const char* name) { return (uintptr_t)Interfaces::getInterface<void*>(file, name); }
        uintptr_t getAbsoluteAddress(uintptr_t ptr, int offset, int size) { return (uintptr_t)Memory::getAbsoluteAddress(ptr, offset, size); }
        /* patternScan */
    }

    namespace Pred {
        void start(CUserCmd cmd) { Prediction::startPrediction(&cmd); }
        void end() { Prediction::endPrediction(); }
        int commandsPredicted() { return Interfaces::prediction->Split->nCommandsPredicted; }
        void restoreToFrame(int frame) { Interfaces::restoreEntityToPredictedFrame(Interfaces::prediction, 0, frame); }
    }

    namespace Beam {
        void createBeam(Vector start, Vector end, const char* modelName, ImColor color, float dieTime, float width, float amplitude) {
            BeamInfo beamInfo;

            beamInfo.start = start;
            beamInfo.end = end;

            beamInfo.modelName = modelName;
            beamInfo.modelIndex = Interfaces::modelInfo->getModelIndex(modelName);
            beamInfo.haloName = nullptr;
            beamInfo.haloIndex = -1;
            beamInfo.haloScale = 0;

            beamInfo.red = 255.0f * color.Value.x;
            beamInfo.green = 255.0f * color.Value.y;
            beamInfo.blue = 255.0f * color.Value.z;
            beamInfo.brightness = 255.0f * color.Value.w;
            
            beamInfo.type = 0;
            beamInfo.life = 0.0f;
            beamInfo.amplitude = amplitude;
            beamInfo.segments = -1;
            beamInfo.renderable = true;
            beamInfo.speed = 0.5f;
            beamInfo.startFrame = 0;
            beamInfo.frameRate = 0.0f;
            beamInfo.width = width;
            beamInfo.endWidth = width;
            beamInfo.flags = 0;
            beamInfo.fadeLength = 0.0f;

            if (const auto beam = Interfaces::renderBeams->createBeamPoints(&beamInfo)) {
                constexpr auto FBEAM_FOREVER = 0x4000;
                beam->flags &= ~FBEAM_FOREVER;
                beam->die = Interfaces::globals->curtime + dieTime;
            }
        }

        void ringBeam(Vector center, float ringStartRadius, float ringEndRadius, const char* modelName, ImColor color, float life, float width) {
            BeamInfo beamInfo;
            beamInfo.type = 7;
            beamInfo.modelName = modelName;
            beamInfo.modelIndex = Interfaces::modelInfo->getModelIndex(modelName);
            beamInfo.haloName = nullptr;
            beamInfo.haloIndex = -1;
            beamInfo.haloScale = 0;
            beamInfo.life = life;
            beamInfo.width = width;
            beamInfo.fadeLength = 1.0f;
            beamInfo.amplitude = 0.f;
            beamInfo.brightness = 255 * color.Value.w;
            beamInfo.speed = 0;
            beamInfo.startFrame = 0;
            beamInfo.frameRate = 1;
            beamInfo.red = 255 * color.Value.x;
            beamInfo.green = 255 * color.Value.y;
            beamInfo.blue = 255 * color.Value.z;
            beamInfo.segments = 1;
            beamInfo.renderable = true;
            beamInfo.flags = 0;
            beamInfo.ringCenter = center;
            beamInfo.ringStartRadius = ringStartRadius;
            beamInfo.ringEndRadius = ringEndRadius;

            Interfaces::renderBeams->createBeamRingPoints(&beamInfo);
        }
    }

    namespace EntityList {
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
        
        int getLocalPlayer() { return Interfaces::engine->getLocalPlayer(); }
        int getIndexForUserID(int i) { return Interfaces::engine->getPlayerForUserID(i); }
    }

    namespace Panorama {
        void executeScript(const char* script, const char* xmlContext) {
            IUIPanel* root = Interfaces::panorama->getRoot();
            if (root)
                Interfaces::panorama->AccessUIEngine()->RunScript(root, script, xmlContext, 8, 10, false);
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
            int temp = CONFIGFLOAT(configVarName);
            ImGui::SliderInt(labelBuf, &temp, min, max, format);
            CONFIGFLOAT(configVarName) = temp;
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
        void keybinder(const char* configVarName) {
            char labelBuf[64] = "##";
            strcat(labelBuf, configVarName);
            drawKeyBinder(&CONFIGBIND(configVarName));
        }
        bool isKeybinderDown(const char* configVarName) { return isKeyBinderPressed(&CONFIGBIND(configVarName)); }
    }

    namespace Draw {
        void rectangle(ImVec2 min, ImVec2 max, ImColor color, float thickness)                          { curDrawList->AddRect(min, max, color, 0, 0, thickness); }
        void rectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float thickness, float rounding)   { curDrawList->AddRect(min, max, color, rounding, 0, thickness); }
        void filledRectangle(ImVec2 min, ImVec2 max, ImColor color)                                     { curDrawList->AddRectFilled(min, max, color); }
        void filledRectangleRounded(ImVec2 min, ImVec2 max, ImColor color, float rounding)              { curDrawList->AddRectFilled(min, max, color, rounding); }
        void gradientFilledRectangle(ImVec2 min, ImVec2 max, ImColor tl, ImColor tr, ImColor bl, ImColor br) { curDrawList->AddRectFilledMultiColor(min, max, tl, tr, br, bl); }
        
        void line(ImVec2 p1, ImVec2 p2, ImColor color, float thickness) { curDrawList->AddLine(p1, p2, color, thickness); }

        void circle(ImVec2 center, float radius, ImColor color, float thickness)                        { curDrawList->AddCircle(center, radius, color, 0, thickness); }
        void filledCircle(ImVec2 center, float radius, ImColor color)                                   { curDrawList->AddCircleFilled(center, radius, color); }

        ImVec2 calcTextSize(const char* text)                                                           { return ImGui::CalcTextSize(text); }
        void text(ImVec2 pos, ImColor color, const char* text)                                          { curDrawList->AddText(pos, color, text); }
        void shadowText(ImVec2 pos, ImColor color, const char* text) {
            curDrawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0), text);
            curDrawList->AddText(pos, color, text);
        }
        void outlineText(ImVec2 pos, ImColor color, const char* text) {
            curDrawList->AddText(ImVec2(pos.x - 1, pos.y), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
            curDrawList->AddText(ImVec2(pos.x + 1, pos.y), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
            curDrawList->AddText(ImVec2(pos.x, pos.y - 1), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
            curDrawList->AddText(ImVec2(pos.x, pos.y + 1), ImColor(0.f, 0.f, 0.f, color.Value.w / 2), text);
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
        .addFunction("print", print)
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
                .addFunction("distTo", &Vector::DistTo)
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
                .addFunction("sane", &LuaEntity::sane)
                .addFunction("alive", &LuaEntity::alive)
                .addFunction("onground", &LuaEntity::onground)
                .addFunction("movetype", &LuaEntity::movetype)
                .addFunction("index", &LuaEntity::index)
                .addFunction("origin", &LuaEntity::origin)
                .addFunction("velocity", &LuaEntity::velocity)
                .addFunction("classID", &LuaEntity::classID)
                .addFunction("networkName", &LuaEntity::networkName)
                .addFunction("getBBox", &LuaEntity::getBBox)
                .addFunction("setMins", &LuaEntity::setMins)
                .addFunction("setMaxs", &LuaEntity::setMaxs)
                .addFunction("onPreDataChanged", &LuaEntity::onPreDataChanged)
                .addFunction("onDataChanged", &LuaEntity::onDataChanged)
                .addFunction("preDataUpdate", &LuaEntity::preDataUpdate)
                .addFunction("postDataUpdate", &LuaEntity::postDataUpdate)
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
            .beginClass<LuaClientClass>("ClientClass")
                .addFunction("exists", &LuaClientClass::exists)
                .addFunction("create", &LuaClientClass::create)
                .addFunction("name", &LuaClientClass::name)
                .addFunction("next", &LuaClientClass::next)
            .endClass()
            .beginClass<LuaConvar>("Convar")
                .addFunction("ffiPtr", &LuaConvar::ffiPtr)
                .addFunction("getFloat", &LuaConvar::getFloat)
                .addFunction("getInt", &LuaConvar::getInt)
                .addFunction("setString", &LuaConvar::setString)
                .addFunction("setFloat", &LuaConvar::setFloat)
                .addFunction("setInt", &LuaConvar::setInt)
            .endClass()
            .beginClass<LuaGameEvent>("GameEvent")
                .addFunction("ffiPtr", &LuaGameEvent::ffiPtr)
                .addFunction("getBool", &LuaGameEvent::getBool)
                .addFunction("getFloat", &LuaGameEvent::getFloat)
                .addFunction("getInt", &LuaGameEvent::getInt)
                .addFunction("getName", &LuaGameEvent::getName)
                .addFunction("getPtr", &LuaGameEvent::getPtr)
                .addFunction("getString", &LuaGameEvent::getString)
                .addFunction("getUint64", &LuaGameEvent::getUint64)
                .addFunction("getWString", &LuaGameEvent::getWString)
            .endClass()
            .beginNamespace("eclipse")
                .addFunction("registerHook", Eclipse::registerHook)
                .addFunction("setCmd", Eclipse::setCmd)
                .addFunction("getConvar", Eclipse::getConvar)
                .addFunction("addMaterial", Chams::addMaterial)
                .addFunction("removeMaterial", Chams::removeMaterial)
                .addFunction("worldToScreen", Eclipse::_worldToScreen)
                .addFunction("getAllClientClasses", Eclipse::getAllClientClasses)
                .addFunction("addEventListener", Eclipse::addEventListener)
                .addFunction("clientCmd", Eclipse::clientCmd)
                .addFunction("isInGame", Eclipse::isInGame)
                .addFunction("startMovementFix", Eclipse::startMovementFix_)
                .addFunction("endMovementFix", Eclipse::endMovementFix_)
            .endNamespace()
            .beginNamespace("globals")
                .addFunction("registerHook", Eclipse::registerHook)
                .addFunction("setCmd", Eclipse::setCmd)
                .addFunction("getConvar", Eclipse::getConvar)
                .addFunction("addMaterial", Chams::addMaterial)
                .addFunction("removeMaterial", Chams::removeMaterial)
                .addFunction("worldToScreen", Eclipse::_worldToScreen)
                .addFunction("getAllClientClasses", Eclipse::getAllClientClasses)
                .addFunction("addEventListener", Eclipse::addEventListener)
                .addFunction("clientCmd", Eclipse::clientCmd)
                .addFunction("isInGame", Eclipse::isInGame)
            .endNamespace()
            .beginNamespace("memory")
                .addFunction("getInterface", Mem::getInterface)
                .addFunction("getAbsoluteAddress", Mem::getAbsoluteAddress)
                .addFunction("patternScan", Memory::patternScan)
            .endNamespace()
            .beginNamespace("prediction")
                .addFunction("start", Pred::start)
                .addFunction("end_", Pred::end)
                .addFunction("restoreToFrame", Pred::restoreToFrame)
                .addFunction("commandsPredicted", Pred::commandsPredicted)
            .endNamespace()
            .beginNamespace("beam")
                .addFunction("createBeam", Beam::createBeam)
                .addFunction("ringBeam", Beam::ringBeam)
            .endNamespace()
            .beginNamespace("entitylist")
                .addFunction("getEntity", EntityList::getEntity)
                .addFunction("getEntities", EntityList::getEntities)
                .addFunction("getEntitiesByClassID", EntityList::getEntitiesByClassID)
                .addFunction("getLocalPlayer", EntityList::getLocalPlayer)
                .addFunction("getIndexForUserID", EntityList::getIndexForUserID)
            .endNamespace()
            .beginNamespace("panorama")
                .addFunction("executeScript", Panorama::executeScript)
            .endNamespace()
            .beginNamespace("ui")
                .addFunction("getMenuPos", UI::getMenuPos)
                .addFunction("getMenuSize", UI::getMenuSize)
                .addFunction("isMenuOpen", UI::isMenuOpen)
                .addFunction("getConfigBool", UI::getConfigBool)
                .addFunction("setConfigBool", UI::setConfigBool)
                .addFunction("getConfigFloat", UI::getConfigFloat)
                .addFunction("setConfigFloat", UI::setConfigFloat)
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
                .addFunction("keybinder", UI::keybinder)
                .addFunction("isKeybinderDown", UI::isKeybinderDown)
            .endNamespace()
            .beginNamespace("draw")
                .addFunction("rectangle", Draw::rectangle)
                .addFunction("rectangleRounded", Draw::rectangleRounded)
                .addFunction("filledRectangle", Draw::filledRectangle)
                .addFunction("filledRectangleRounded", Draw::filledRectangleRounded)
                .addFunction("gradientFilledRectangle", Draw::gradientFilledRectangle)

                .addFunction("line", Draw::line)

                .addFunction("circle", Draw::circle)
                .addFunction("filledCircle", Draw::filledCircle)

                .addFunction("calcTextSize", Draw::calcTextSize)
                .addFunction("text", Draw::text)
                .addFunction("shadowText", Draw::shadowText)
                .addFunction("outlineText", Draw::outlineText)

                .addFunction("getScreenSize", Draw::getScreenSize)
                .addFunction("deltaTime", Draw::deltaTime)
                .addFunction("HSVtoColor", ImColor::HSV)
                .addFunction("colorConvertFloat4ToU32", ImGui::ColorConvertFloat4ToU32)
            .endNamespace();
    }

    LuaEngine::LuaEngine(std::string filename) {
        std::lock_guard<std::mutex> lock(luaLock);
        curEngineBeingRan = this;
        curEngineBeingRanName = filename.c_str();
        char path[512], filepath[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.eclipse/");
        std::filesystem::create_directory(path);
        strcat(path, "scripts/");
        std::filesystem::create_directory(path);
        strcpy(filepath, path);
        strcat(filepath, filename.c_str());

        state = luaL_newstate();
        luaL_openlibs(state);
        
        bridge(state); // add c++ funcs to lua
        
        luaL_dostring(state, (std::string("package.path = \"") + std::string(path) + std::string("?.lua;\" .. package.path")).c_str());

        if (luaL_dofile(state, filepath)) {
            ERR("lua error: %s", lua_tostring(state, -1));
        }
    }

    void handleHook(const char* hook) {
        std::lock_guard<std::mutex> lock(luaLock);
        for (auto& engine : scripts) {
            curEngineBeingRan = &engine.second;
            curEngineBeingRanName = engine.first.c_str();
            if (engine.second.hooks.find(hook) != engine.second.hooks.end()) {
                int oldStackSize = 0;
                if (strstr(hook, "draw"))
                    oldStackSize = ImGui::GetCurrentContext()->CurrentWindowStack.Size;

                if (strstr(hook, "UI")) {
                    if (ImGui::CollapsingHeader(engine.first.c_str())) {
                        try {
                            engine.second.hooks.at(hook)();
                        }
                        catch (luabridge::LuaException const& e) {
                            ERR("lua error (%s) (%s): %s", engine.first.c_str(), hook, e.what());
                        }
                    }
                    ImGui::Separator();
                }
                else {
                    try { 
                        engine.second.hooks.at(hook)();
                    }
                    catch (luabridge::LuaException const& e) {
                        ERR("lua error (%s) (%s): %s", engine.first.c_str(), hook, e.what());
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
}