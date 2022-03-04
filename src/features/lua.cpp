#define STB_IMAGE_IMPLEMENTATION
#include <GL/gl.h>
#include "lua.hpp"
#include "../interfaces.hpp"
#include "../hooks.hpp"
#include "../util/log.hpp"
#include "../menu/config.hpp"
#include "../menu/menu.hpp"
#include "../menu/imgui/imgui_internal.h"
#include "../menu/imgui/imgui_freetype.h"
#include "../menu/imgui/imgui_impl_opengl3.h"
#include "../menu/imgui/stb_image.h"
#include "../sdk/entity.hpp"
#include "../sdk/math.hpp"
#include "../sdk/netvars.hpp"
#include "chams.hpp"
#include "glow.hpp"

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
        Vector getMins() { return e->nDT_BaseEntity__m_Collision().OBBMins(); }
        Vector getMaxs() { return e->nDT_BaseEntity__m_Collision().OBBMaxs(); }
        bool teammate() {return e->teammate(); }
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

        void setViewAngles(QAngle viewangles) { Interfaces::engine->setViewAngles(viewangles); }

        Vector angleVector(QAngle angle) {
            Vector forward = Vector(0, 0, 0); 
            forward.x = cos(DEG2RAD(angle.x)) * cos(DEG2RAD(angle.y));
            forward.y = cos(DEG2RAD(angle.x)) * sin(DEG2RAD(angle.y));
            forward.z = -sin(DEG2RAD(angle.x));
            return forward;
        }
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
        void createBeam(Vector start, Vector end, const char* modelName, ImColor color, float life, float width, float amplitude) {
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
                beam->die = Interfaces::globals->curtime + life;
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
            return LuaEntity(ent);
        }

        LuaEntity getEntityFromHandle(int i) {
            Entity* ent = (Entity*)Interfaces::entityList->getClientEntity(i & 0xfff);
            return LuaEntity(ent);
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

    namespace TraceRay {
        struct TraceResult {
            float fraction;
            const char* surfaceName;
            int hitgroup;
            int hitbox;
            int contents;
            float slopeAngle;
            LuaEntity entityHit;
        };
        
        TraceResult trace(Vector begin, Vector end, LuaEntity skip, unsigned int mask) {
            TraceFilter filter;
            filter.pSkip = skip.e;

            Trace trace;
            Ray ray;
            ray.Init(begin, end);
            Interfaces::trace->traceRay(ray, mask, &filter, &trace);

            return {trace.fraction, trace.surface.name, trace.hitgroup, trace.hitbox, trace.contents, trace.plane.normal.z, LuaEntity(trace.m_pEntityHit)};
        }
        
        TraceResult traceHull(Vector start, Vector end, Vector min, Vector max, LuaEntity skip, unsigned int mask) {
            TraceFilter filter;
            filter.pSkip = skip.e;

            Trace trace;
            Ray ray;
            ray.Init(start, end, min, max);
            Interfaces::trace->traceRay(ray, mask, &filter, &trace);

            return {trace.fraction, trace.surface.name, trace.hitgroup, trace.hitbox, trace.contents, trace.plane.normal.z, LuaEntity(trace.m_pEntityHit)};
        }

        TraceResult traceHullSimple(Vector start, Vector end, Vector min, Vector max) {
                                                            // solid|opaque|moveable|ignore nodraw
            return traceHull(start, end, min, max, EntityCache::localPlayer, (0x1 | 0x80 | 0x4000 | 0x2000 | 0x40000000));
        }

        TraceResult traceSimple(Vector begin, Vector end) {
                                                            // solid|opaque|moveable|ignore nodraw
            return trace(begin, end, EntityCache::localPlayer, (0x1 | 0x80 | 0x4000 | 0x2000 | 0x40000000));
        }
    }

    namespace Panorama {
        void executeScript(const char* script, const char* xmlContext) {
            IUIPanel* root = Interfaces::panorama->getRoot();
            if (root)
                Interfaces::panorama->AccessUIEngine()->RunScript(root, script, xmlContext, 8, 10, false);
        }
    }

    namespace _Glow {
        bool glowEntity(LuaEntity entity, ImColor color, int style) {
            int index = Interfaces::glowManager->registerGlowObject(entity.e);
            if (index == -1)
                return false;

            Glow::customGlowEntities.emplace_back(entity.index(), index);
            
		    GlowObjectDefinition& glowObject = Interfaces::glowManager->glowObjectDefinitions[index];

            glowObject.renderWhenOccluded = true;
            glowObject.glowAlpha = color.Value.w;
            glowObject.glowStyle = style;
            glowObject.glowColor = {color.Value.x, color.Value.y, color.Value.z};
            return true;
        }

        void createDlight(Vector origin, int index, float dieTime, ImColor color, float radius, float decay) {
            dlight_t* dLight = Interfaces::effects->CL_AllocDlight(index);
            dLight->key = index;
            dLight->color.r = color.Value.x * 255;
            dLight->color.g = color.Value.y * 255;
            dLight->color.b = color.Value.z * 255;
            dLight->color.exponent = true;
            dLight->flags = 0;
            dLight->direction = origin;
            dLight->origin = origin;
            dLight->radius = radius;
            dLight->die = Interfaces::globals->curtime + dieTime;
            dLight->decay = decay;
        }
    }

    namespace UI {
        ImVec2 getMenuPos() { return Menu::windowPos; }
        ImVec2 getMenuSize() { return Menu::windowSize; }
        ImVec2 getCurrentWindowPos() { return ImGui::GetWindowPos(); }
        ImVec2 getCurrentWindowSize() { return ImGui::GetWindowSize(); }
        ImVec2 getMousePos() { return ImGui::GetMousePos(); }
        std::vector<int> getKeysPressed() {
            ImGuiIO io = ImGui::GetIO();
            std::vector<int> keysPressed;
            for (int i = 0; i < sizeof(io.KeysDown); i++) {
                if (io.KeysDown[i])
                    keysPressed.push_back(i);
            }
            return keysPressed;
        }
        int getMousePressed() {
            ImGuiIO io = ImGui::GetIO();
            for (int i = 1; i <= sizeof(io.MouseDown); i++) {
                if (io.MouseDown[i - 1]) {
                    return i;
                }
            }
            return 0;
        }
        bool isMenuOpen() { return Menu::menuOpen; }
        bool getConfigBool(const char* var) { return CONFIGBOOL(var); }
        void setConfigBool(const char* var, bool val) { CONFIGBOOL(var) = val; }
        float getConfigFloat(const char* var) { return CONFIGFLOAT(var); }
        void setConfigFloat(const char* var, float val) { CONFIGFLOAT(var) = val; }
        const char* getConfigStr(const char* var) { return CONFIGSTR(var).c_str(); }
        void setConfigStr(const char* var, const char* val) { CONFIGSTR(var) = val; }
        ImColor getConfigCol(const char* var) { return CONFIGCOL(var); }
        void setConfigCol(const char* var, ImColor val) { CONFIGCOL(var) = val; }
        void beginWindow(const char* title) { ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | (Menu::menuOpen ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs)); }
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
        const char* textInputMultiline(const char* label, const char* configVarName, int height) {
            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            char buf[1024];
            strcpy(buf, CONFIGSTR(configVarName).c_str());
            ImGui::Text("%s", label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::InputTextMultiline(labelBuf, buf, sizeof(buf), ImVec2(ImGui::GetContentRegionAvailWidth(), height));
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
            drawKeyBinder(configVarName);
        }
        bool isKeybinderDown(const char* configVarName) { return isKeyBinderPressed(&CONFIGBIND(configVarName)); }
        int comboBox(const char* label, const char* configVarName, std::vector<const char*> items) {
            ImGui::Text("%s", label);

            char labelBuf[64] = "##";
            strcat(labelBuf, label);
            int temp = CONFIGINT(configVarName);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            ImGui::Combo(labelBuf, &temp, &items[0], items.size());
            CONFIGFLOAT(configVarName) = temp;
            return temp;
        }
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

        ImVec2 getScreenSize() { return ImGui::GetIO().DisplaySize; }

        float deltaTime() { return ImGui::GetIO().DeltaTime; }

        void pushFont(uintptr_t font) { ImGui::PushFont((ImFont*)font); }
        void popFont() { ImGui::PopFont(); }
        uintptr_t loadFont(const char* filename, int size, bool hinting) { 
            char path[512];
            strcpy(path, getenv("HOME"));
            strcat(path, "/.eclipse/");
            std::filesystem::create_directory(path);
            strcat(path, "scripts/");
            strcat(path, filename);
            
            ImFontConfig cfg;
            cfg.FontBuilderFlags = hinting ? 0 : ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_Monochrome;
            
            ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(path, size, &cfg);
		    ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0x0);
            ImGui_ImplOpenGL3_DestroyDeviceObjects();
            return (uintptr_t)font;
        }

        struct LuaImage {
            GLuint image;
            int width;
            int height;
        };

        LuaImage loadImage(const char* filename) {
            char path[512];
            strcpy(path, getenv("HOME"));
            strcat(path, "/.eclipse/");
            std::filesystem::create_directory(path);
            strcat(path, "scripts/");
            strcat(path, filename);

            // Load from file
            int image_width = 0;
            int image_height = 0;
            unsigned char* image_data = stbi_load(path, &image_width, &image_height, NULL, 4);
            if (image_data == NULL)
                return {0, 0, 0};

            // Create a OpenGL texture identifier
            GLuint image_texture;
            glGenTextures(1, &image_texture);
            glBindTexture(GL_TEXTURE_2D, image_texture);

            // Setup filtering parameters for display
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

            // Upload pixels into texture
        #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        #endif
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            stbi_image_free(image_data);

            return {image_texture, image_width, image_height};
        }

        void drawImage(LuaImage image, ImVec2 min, ImVec2 max) {
            if (!image.image) {
                WARN("%s - image not found.", curEngineBeingRanName);
                return;
            }
            
            curDrawList->AddImage((void*)(intptr_t)image.image, min, max);
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
            .beginClass<Vector>("Vector")
                .addConstructor<void (*) (float, float, float)>()
                .addProperty("x", &Vector::x)
                .addProperty("y", &Vector::y)
                .addProperty("z", &Vector::z)
                .addFunction("length", &Vector::Length)
                .addFunction("length2D", &Vector::Length2D)
                .addFunction("distTo", &Vector::DistTo)
            .endClass()
            .beginClass<ImColor>("Color")
                .addConstructor<void (*) (float, float, float, float)>()
                .addProperty("value", &ImColor::Value)
            .endClass()
            .beginClass<QAngle>("QAngle")
                .addConstructor<void (*) (float, float, float)>()
                .addProperty("x", &QAngle::x)
                .addProperty("y", &QAngle::y)
                .addProperty("z", &QAngle::z)
                .addFunction("length", &QAngle::Length)
            .endClass()
            .beginClass<CUserCmd>("UserCmd")
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
                .addFunction("getMins", &LuaEntity::getMins)
                .addFunction("getMaxs", &LuaEntity::getMaxs)
                .addFunction("onPreDataChanged", &LuaEntity::onPreDataChanged)
                .addFunction("onDataChanged", &LuaEntity::onDataChanged)
                .addFunction("preDataUpdate", &LuaEntity::preDataUpdate)
                .addFunction("postDataUpdate", &LuaEntity::postDataUpdate)
                .addFunction("teammate", &LuaEntity::teammate)
                .addFunction("getPropBool", &LuaEntity::getProp<bool>)
                .addFunction("getPropInt", &LuaEntity::getProp<int>)
                .addFunction("getPropFloat", &LuaEntity::getProp<float>)
                .addFunction("getPropPtr", &LuaEntity::getProp<unsigned int>)
                .addFunction("getPropQAngle", &LuaEntity::getProp<QAngle>)
                .addFunction("getPropVector", &LuaEntity::getProp<Vector>)
                .addFunction("getPropStr", &LuaEntity::getProp<const char*>)
                .addFunction("setPropBool", &LuaEntity::setProp<bool>)
                .addFunction("setPropInt", &LuaEntity::setProp<int>)
                .addFunction("setPropFloat", &LuaEntity::setProp<float>)
                .addFunction("setPropPtr", &LuaEntity::setProp<unsigned int>)
                .addFunction("setPropQAngle", &LuaEntity::setProp<QAngle>)
                .addFunction("setPropVector", &LuaEntity::setProp<Vector>)
                .addFunction("setPropStr", &LuaEntity::setProp<const char*>)
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
                .addFunction("setStr", &LuaConvar::setString)
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
                .addFunction("startMovementFix", startMovementFix)
                .addFunction("endMovementFix", endMovementFix)
                .addFunction("setViewAngles", Eclipse::setViewAngles)
                .addFunction("angleVector", Eclipse::angleVector)
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
                .addFunction("getEntityFromHandle", EntityList::getEntityFromHandle)
                .addFunction("getEntities", EntityList::getEntities)
                .addFunction("getEntitiesByClassID", EntityList::getEntitiesByClassID)
                .addFunction("getLocalPlayer", EntityList::getLocalPlayer)
                .addFunction("getIndexForUserID", EntityList::getIndexForUserID)
            .endNamespace()
            .beginNamespace("trace")
                .beginClass<TraceRay::TraceResult>("TraceResult")
                    .addProperty("contents", &TraceRay::TraceResult::contents)
                    .addProperty("entityHit", &TraceRay::TraceResult::entityHit)
                    .addProperty("fraction", &TraceRay::TraceResult::fraction)
                    .addProperty("hitbox", &TraceRay::TraceResult::hitbox)
                    .addProperty("hitgroup", &TraceRay::TraceResult::hitgroup)
                    .addProperty("surfaceName", &TraceRay::TraceResult::surfaceName)
                    .addProperty("slopeAngle", &TraceRay::TraceResult::slopeAngle)
                .endClass()
                .addFunction("trace", TraceRay::trace)
                .addFunction("traceSimple", TraceRay::traceSimple)
                .addFunction("traceHull", TraceRay::traceHull)
                .addFunction("traceHullSimple", TraceRay::traceHullSimple)
            .endNamespace()
            .beginNamespace("panorama")
                .addFunction("executeScript", Panorama::executeScript)
            .endNamespace()
            .beginNamespace("glow")
                .addFunction("glowEntity", _Glow::glowEntity)
                .addFunction("createDlight", _Glow::createDlight)
            .endNamespace()
            .beginNamespace("ui")
                .addFunction("getMenuPos", UI::getMenuPos)
                .addFunction("getMenuSize", UI::getMenuSize)
                .addFunction("getCurrentWindowPos", UI::getCurrentWindowPos)
                .addFunction("getCurrentWindowSize", UI::getCurrentWindowSize)
                .addFunction("getMousePos", UI::getMousePos)
                .addFunction("getKeysPressed", UI::getKeysPressed)
                .addFunction("getMousePressed", UI::getMousePressed)
                .addFunction("isMenuOpen", UI::isMenuOpen)
                .addFunction("getConfigBool", UI::getConfigBool)
                .addFunction("setConfigBool", UI::setConfigBool)
                .addFunction("getConfigFloat", UI::getConfigFloat) // dep
                .addFunction("setConfigFloat", UI::setConfigFloat) // dep
                .addFunction("getConfigNumber", UI::getConfigFloat)
                .addFunction("setConfigNumber", UI::setConfigFloat)
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
                .addFunction("comboBox", UI::comboBox)
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

                .addFunction("pushFont", Draw::pushFont)
                .addFunction("popFont", Draw::popFont)
                .addFunction("loadFont", Draw::loadFont)

                .beginClass<Draw::LuaImage>("Image")
                    .addProperty("image", &Draw::LuaImage::image)
                    .addProperty("width", &Draw::LuaImage::width)
                    .addProperty("height", &Draw::LuaImage::height)
                .endClass()
                .addFunction("loadImage", Draw::loadImage)
                .addFunction("drawImage", Draw::drawImage)
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
                INFO("running %s for %s", hook, engine.first.c_str());
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
                INFO("ran %s for %s", hook, engine.first.c_str());
            }
        }
    }
}