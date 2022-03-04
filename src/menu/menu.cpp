#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_freetype.h"
#include "imgui/GL/gl3w.h"
#include "menu.hpp"
#include "fonts.hpp"
#include "config.hpp"
#include "../features/lua.hpp"
#include "../features/discordrpc.hpp"
#include "../features/esp.hpp"
#include "../features/chams.hpp"
#include "../features/glow.hpp"
#include "../features/skinchanger.hpp"
#include "../sdk/entity.hpp"
#include "../features/luabridge/LuaBridge.h"
#include "keybinders.hpp"
#include <GL/gl.h>

#define BEGINGROUPBOX(name, size) ImGui::BeginChild(name, size, true); ImGui::TextColored(ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight())) ? ImVec4(1.f, 1.f, 1.f, 1.f) : ImVec4(0.8f, 0.8f, 0.8f, 1.f), name); ImGui::Separator()
#define ENDGROUPBOX() ImGui::EndChild()
#define CHECKBOX(name, var) ImGui::Checkbox(name, var)
#define COMBOBOX(name, var, array, size) ImGui::Text(name); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth()); ImGui::Combo("##" name, var, array, size);
#define COLORPICKER(name, var) ImGui::ColorEdit4(name, (float*)&var.Value, ImGuiColorEditFlags_NoInputs);
#define SLIDERINT(name, var, min, max, format) ImGui::Text("%s", name); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth()); ImGui::SliderInt("##" name, var, min, max, format);
#define SLIDERFLOAT(name, var, min, max, format) ImGui::Text("%s", name); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth()); ImGui::SliderFloat("##" name, var, min, max, format);

namespace Menu {
    void onPollEvent(SDL_Event* event, const int result) {
        if (result && ImGui_ImplSDL2_ProcessEvent(event) && menuOpen) {
            event->type = 0;
        }
    }

    /*bool loggedIn = false;

    void loginScreen() {
        ImGui::GetIO().MouseDrawCursor = true;
        ImGui::Begin("eclipse.wtf login", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::GetForegroundDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 1), ImColor(0.09f, 0.09f, 0.09f));
        ImGui::BeginChild("content", ImVec2(212, 134), true);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.16f, 1.00f));

        ImGui::BeginChild("login", ImVec2(200, 122), true);

        ImGui::Text("eclipse.wtf login");
        ImGui::Separator();

        ImGui::Text("Username:");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        static char tempUsername[64] = "";
        ImGui::InputText("##username", tempUsername, sizeof(tempUsername));

        ImGui::Text("Password:");
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
        static char tempPasswd[64] = "";
        ImGui::InputText("##password", tempPasswd, sizeof(tempPasswd), ImGuiInputTextFlags_Password);

        if (ImGui::Button("login", ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
            if (Protection::login(tempUsername, tempPasswd))
                loggedIn = true;

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::End();
    }*/

    void onSwapWindow(SDL_Window* window) {
        if (!initialised) {
            gl3wInit();
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImFontConfig fontConfig;
            fontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_Monochrome;
            menuFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(tahoma_compressed_data, tahoma_compressed_size, 14, &fontConfig);

            weaponFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(&weaponIcons[0], weaponIcons.size(), 14);
		    ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0x0);

            ImGui::GetStyle().WindowPadding = ImVec2(6, 6);
            ImGui::GetStyle().FramePadding = ImVec2(4, 0);
            ImGui::GetStyle().WindowRounding = 1;
            ImGui::GetStyle().ChildRounding = 1;
            ImGui::GetStyle().FrameRounding = 1;
            ImGui::GetStyle().WindowBorderSize = 0;
            ImGui::GetStyle().FrameBorderSize = 1;
            ImGui::GetStyle().ScrollbarRounding = 0;
            ImGui::GetStyle().ScrollbarSize = 3;
            ImGui::GetStyle().WindowMinSize = ImVec2(4, 4);

            ImVec4* colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_WindowBg]               = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_Border]                 = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);



            ImGui_ImplOpenGL3_Init("#version 100");
            ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
            Config::refreshConfigList();
            Lua::refreshLuaList();
            initialised = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        static float timeSinceLastTick = 0.f;
        timeSinceLastTick += ImGui::GetIO().DeltaTime;
        if (timeSinceLastTick > 5.f) {
            DiscordRPC::tick(); // tick every 5 seconds for discord RPC
            timeSinceLastTick = 0.f;
        }

        /*if (!loggedIn) {
            loginScreen();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            return;
        }*/

        ESP::draw(ImGui::GetBackgroundDrawList());
        Visuals::watermark();

        Lua::curDrawList = ImGui::GetBackgroundDrawList();
        Lua::handleHook("draw");

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);

        if (ImGui::IsKeyPressed(SDL_SCANCODE_INSERT, false)) {
            menuOpen = !menuOpen;
        }

        if (menuOpen) {
            ImGui::GetIO().MouseDrawCursor = true;
            ImGui::Begin("eclipse", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            windowPos = ImGui::GetWindowPos();
            windowSize = ImGui::GetWindowSize();
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 1), ImColor(0.09f, 0.09f, 0.09f));

            ImGui::BeginChild("content", ImVec2(450, 400), true);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.16f, 1.00f));
            switch (curTab) {
                case 0: {
                    ImGui::SetCursorPos(ImVec2(6, 6));

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::BeginChild("tabs", ImVec2(ImGui::GetWindowContentRegionWidth(), 30), true);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 0.8, 0.8));
                    static int curSubTab = 0;
                    static int lastCurSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("default", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PopStyleColor();
                    ImGui::PushFont(weaponFont);
                    ImGui::SameLine();
                    if (lastCurSubTab == 1) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("G", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 1;
                    if (lastCurSubTab == 1) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 2) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("A", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 2;
                    if (lastCurSubTab == 2) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 3) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("W", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 3;
                    if (lastCurSubTab == 3) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 4) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("a", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 4;
                    if (lastCurSubTab == 4) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 5) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("Z", ImVec2(ImGui::GetWindowWidth()/6, ImGui::GetWindowHeight()))) curSubTab = 5;
                    if (lastCurSubTab == 5) ImGui::PopStyleColor();
                    lastCurSubTab = curSubTab;
                    ImGui::PopFont();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                    ImGui::EndChild();
                    ImGui::PopStyleVar();

                    ImGui::SetCursorPos(ImVec2(6, 42));

                    switch (curSubTab) {
                        case 0: { // default
                            BEGINGROUPBOX("default legitbot", ImVec2(438, 214));
                            ImGui::Text("fov");
                            ImGui::SameLine();
                            drawKeyBinder("legitbot key");
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30);
                            ImGui::SliderFloat("##fov", &CONFIGFLOAT("default fov"), 0, 180, "%.2f");
                            SLIDERFLOAT("smoothing", &CONFIGFLOAT("default smoothing"), 0, 100, "%.2f");
                            ENDGROUPBOX();
                            break;
                        }
                        case 1: { // pistol
                            BEGINGROUPBOX("pistols", ImVec2(438, 214));
                            CHECKBOX("override", &CONFIGBOOL("pistol override"));
                            if (CONFIGBOOL("pistol override")) {
                                ImGui::SameLine();
                                drawKeyBinder("legitbot key");
                                ImGui::Text("pistol fov");
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30); 
                                ImGui::SliderFloat("##fov", &CONFIGFLOAT("pistol fov"), 0, 180, "%.2f");
                                SLIDERFLOAT("smoothing", &CONFIGFLOAT("pistol smoothing"), 0, 100, "%.2f");
                            }
                            ENDGROUPBOX();
                            break;
                        }
                        case 2: { // heavy pistol
                            BEGINGROUPBOX("heavy pistols", ImVec2(438, 214));
                            CHECKBOX("override", &CONFIGBOOL("heavy pistol override"));
                            if (CONFIGBOOL("heavy pistol override")) {
                                ImGui::SameLine();
                                drawKeyBinder("legitbot key");
                                ImGui::Text("heavy pistol fov");
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30); 
                                ImGui::SliderFloat("##fov", &CONFIGFLOAT("heavy pistol fov"), 0, 180, "%.2f");
                                SLIDERFLOAT("smoothing", &CONFIGFLOAT("heavy pistol smoothing"), 0, 100, "%.2f");
                            }
                            ENDGROUPBOX();
                            break;
                        }
                        case 3: { // rifle
                            BEGINGROUPBOX("rifles", ImVec2(438, 214));
                            CHECKBOX("override", &CONFIGBOOL("rifle override"));
                            if (CONFIGBOOL("rifle override")) {
                                ImGui::SameLine();
                                drawKeyBinder("legitbot key");
                                ImGui::Text("rifle fov");
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30); 
                                ImGui::SliderFloat("##fov", &CONFIGFLOAT("rifle fov"), 0, 180, "%.2f");
                                SLIDERFLOAT("smoothing", &CONFIGFLOAT("rifle smoothing"), 0, 100, "%.2f");
                            }
                            ENDGROUPBOX();
                            break;
                        }
                        case 4: { // scout
                            BEGINGROUPBOX("scout", ImVec2(438, 214));
                            CHECKBOX("override", &CONFIGBOOL("scout override"));
                            if (CONFIGBOOL("scout override")) {
                                ImGui::SameLine();
                                drawKeyBinder("legitbot key");
                                ImGui::Text("scout fov");
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30); 
                                ImGui::SliderFloat("##fov", &CONFIGFLOAT("scout fov"), 0, 180, "%.2f");
                                SLIDERFLOAT("smoothing", &CONFIGFLOAT("scout smoothing"), 0, 100, "%.2f");
                            }
                            ENDGROUPBOX();
                            break;
                        }
                        case 5: { // AWP
                            BEGINGROUPBOX("AWP", ImVec2(438, 214));
                            CHECKBOX("override", &CONFIGBOOL("AWP override"));
                            if (CONFIGBOOL("AWP override")) {
                                ImGui::SameLine();
                                drawKeyBinder("legitbot key");
                                ImGui::Text("AWP fov");
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30); 
                                ImGui::SliderFloat("##fov", &CONFIGFLOAT("AWP fov"), 0, 180, "%.2f");
                                SLIDERFLOAT("smoothing", &CONFIGFLOAT("AWP smoothing"), 0, 100, "%.2f");
                            }
                            ENDGROUPBOX();
                            break;
                        }
                    }

                    ImGui::SetCursorPos(ImVec2(6, 262));

                    BEGINGROUPBOX("other", ImVec2(216, 131));
                        SLIDERFLOAT("backtrack", &CONFIGFLOAT("backtrack"), 0.f, 0.2f, "%.4f");
                        Chams::chamsCombo("backtrack chams material", CONFIGSTR("backtrack chams material"), CONFIGCOL("backtrack chams color"));
                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(228, 262));

                    BEGINGROUPBOX("triggerbot", ImVec2(216, 131));
                    
                    ENDGROUPBOX();
                    break;
                }
                case 1: {
                    ImGui::SetCursorPos(ImVec2(6, 6));

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::BeginChild("tabs", ImVec2(ImGui::GetWindowContentRegionWidth(), 30), true);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 0.8, 0.8));
                    static int curSubTab = 0;
                    static int lastCurSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("enemies", ImVec2(ImGui::GetWindowWidth()/3, ImGui::GetWindowHeight()))) curSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 1) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("teammates", ImVec2(ImGui::GetWindowWidth()/3, ImGui::GetWindowHeight()))) curSubTab = 1;
                    if (lastCurSubTab == 1) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 2) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("world", ImVec2(ImGui::GetWindowWidth()/3, ImGui::GetWindowHeight()))) curSubTab = 2;
                    if (lastCurSubTab == 2) ImGui::PopStyleColor();
                    lastCurSubTab = curSubTab;
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                    ImGui::EndChild();
                    ImGui::PopStyleVar();

                    ImGui::SetCursorPos(ImVec2(6, 42));

                    switch (curSubTab) {
                        case 0: { // Enemy
                            BEGINGROUPBOX("ESP", ImVec2(216, 351));
                                COLORPICKER("##box color", CONFIGCOL("enemy esp box color"));
                                ImGui::SameLine();
                                CHECKBOX("box", &CONFIGBOOL("enemy esp box"));

                                COLORPICKER("##name color", CONFIGCOL("enemy esp name color"));
                                ImGui::SameLine();
                                CHECKBOX("name", &CONFIGBOOL("enemy esp name"));

                                COLORPICKER("##healthbar color", CONFIGCOL("enemy esp healthbar color"));
                                ImGui::SameLine();
                                CHECKBOX("healthbar", &CONFIGBOOL("enemy esp healthbar"));

                                CHECKBOX("visible only", &CONFIGBOOL("enemy visible only"));
                                ImGui::Separator();
                                COLORPICKER("##glow enemy color", CONFIGCOL("glow enemy color"));
                                ImGui::SameLine();
                                CHECKBOX("glow", &CONFIGBOOL("glow enemy"));
                                int glowStyle = CONFIGINT("glow enemy style");
                                COMBOBOX("glow style", &glowStyle, Glow::styles, IM_ARRAYSIZE(Glow::styles));
                                CONFIGFLOAT("glow enemy style") = glowStyle;
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 42));

                            BEGINGROUPBOX("chams", ImVec2(216, 351));
                                Chams::chamsCombo("chams material", CONFIGSTR("enemy chams material"), CONFIGCOL("enemy chams color"));
                                Chams::chamsCombo("overlay chams material", CONFIGSTR("enemy overlay chams material"), CONFIGCOL("enemy overlay chams color"));
                                Chams::chamsCombo("ignorez chams material", CONFIGSTR("enemy ignorez chams material"), CONFIGCOL("enemy ignorez chams color"));
                            ENDGROUPBOX();
                            break;
                        }
                        case 1: { // Team
                            BEGINGROUPBOX("ESP", ImVec2(216, 351));
                                COLORPICKER("##box color", CONFIGCOL("team esp box color"));
                                ImGui::SameLine();
                                CHECKBOX("box", &CONFIGBOOL("team esp box"));

                                COLORPICKER("##name color", CONFIGCOL("team esp name color"));
                                ImGui::SameLine();
                                CHECKBOX("name", &CONFIGBOOL("team esp name"));
                            
                                COLORPICKER("##healthbar color", CONFIGCOL("team esp healthbar color"));
                                ImGui::SameLine();
                                CHECKBOX("healthbar", &CONFIGBOOL("team esp healthbar"));

                                CHECKBOX("visible only", &CONFIGBOOL("teammate visible only"));

                                ImGui::Separator();
                                COLORPICKER("##glow teammate color", CONFIGCOL("glow teammate color"));
                                ImGui::SameLine();
                                CHECKBOX("glow", &CONFIGBOOL("glow teammate"));
                                int glowStyle = CONFIGINT("glow teammate style");
                                COMBOBOX("glow style", &glowStyle, Glow::styles, IM_ARRAYSIZE(Glow::styles));
                                CONFIGFLOAT("glow teammate style") = glowStyle;
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 42));

                            BEGINGROUPBOX("chams", ImVec2(216, 351));
                                Chams::chamsCombo("chams material", CONFIGSTR("team chams material"), CONFIGCOL("team chams color"));
                                Chams::chamsCombo("overlay chams material", CONFIGSTR("team overlay chams material"), CONFIGCOL("team overlay chams color"));
                                Chams::chamsCombo("ignorez chams material", CONFIGSTR("team ignorez chams material"), CONFIGCOL("team ignorez chams color"));
                            ENDGROUPBOX();
                            break;
                        }
                        case 2: { /* World */
                            BEGINGROUPBOX("world", ImVec2(216, 154));
                                SLIDERFLOAT("nightmode", &CONFIGFLOAT("nightmode"), 0, 1, "%.2f");

                                CHECKBOX("remove 3d skybox", &CONFIGBOOL("remove 3d skybox"));
                                static int curSkyBoxSelected = CONFIGINT("skybox");
                                COMBOBOX("skybox", &curSkyBoxSelected, Visuals::skyboxes, IM_ARRAYSIZE(Visuals::skyboxes));
                                CONFIGFLOAT("skybox") = curSkyBoxSelected;
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(6, 202));

                            BEGINGROUPBOX("item ESP", ImVec2(216, 191));
                                COLORPICKER("##box color", CONFIGCOL("weapon box color"));
                                ImGui::SameLine();
                                CHECKBOX("weapon box", &CONFIGBOOL("weapon box"));

                                COLORPICKER("##weapon name color", CONFIGCOL("weapon name color"));
                                ImGui::SameLine();
                                CHECKBOX("weapon name", &CONFIGBOOL("weapon name"));

                                CHECKBOX("dynamic transparency", &CONFIGBOOL("dynamic weapon transparency"));

                                COLORPICKER("##glow weapon color", CONFIGCOL("glow weapon color"));
                                ImGui::SameLine();
                                CHECKBOX("weapon glow", &CONFIGBOOL("glow weapon"));

                                int glowStyle = CONFIGINT("glow weapon style");
                                COMBOBOX("weapon glow style", &glowStyle, Glow::styles, IM_ARRAYSIZE(Glow::styles));
                                CONFIGFLOAT("glow weapon style") = glowStyle;

                                ImGui::Separator();

                                COLORPICKER("##glow grenade color", CONFIGCOL("glow grenade color"));
                                ImGui::SameLine();
                                CHECKBOX("grenade glow", &CONFIGBOOL("glow grenade"));

                                glowStyle = CONFIGINT("glow grenade style");
                                COMBOBOX("grenade glow style", &glowStyle, Glow::styles, IM_ARRAYSIZE(Glow::styles));
                                CONFIGFLOAT("glow grenade style") = glowStyle;

                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 42));

                            BEGINGROUPBOX("self", ImVec2(216, 351));
                            
                            ENDGROUPBOX();
                            break;
                        }
                    }
                    break;
                }
                case 2: {
                    ImGui::SetCursorPos(ImVec2(6, 6));

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::BeginChild("tabs", ImVec2(ImGui::GetWindowContentRegionWidth(), 30), true);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 0.8, 0.8));
                    static int curSubTab = 0;
                    static int lastCurSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("misc", ImVec2(ImGui::GetWindowWidth()/2, ImGui::GetWindowHeight()))) curSubTab = 0;
                    if (lastCurSubTab == 0) ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (lastCurSubTab == 1) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    if (ImGui::Button("skins", ImVec2(ImGui::GetWindowWidth()/2, ImGui::GetWindowHeight()))) curSubTab = 1;
                    if (lastCurSubTab == 1) ImGui::PopStyleColor();
                    lastCurSubTab = curSubTab;
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                    ImGui::EndChild();
                    ImGui::PopStyleVar();

                    ImGui::SetCursorPos(ImVec2(6, 42));
                    
                    switch (curSubTab) {
                        case 0: { /* misc */
                            BEGINGROUPBOX("misc", ImVec2(438, 214));
                                CHECKBOX("auto accept", &CONFIGBOOL("auto accept"));
                                CHECKBOX("rank reveal", &CONFIGBOOL("rank reveal"));
                                bool alwaysFalse = false;
                                CHECKBOX("insecure bypass", Interfaces::insecure ? &CONFIGBOOL("insecure bypass") : &alwaysFalse);
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(6, 262));

                            BEGINGROUPBOX("clantag", ImVec2(216, 131));

                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 262));

                            BEGINGROUPBOX("movement", ImVec2(216, 131));
                                CHECKBOX("bhop", &CONFIGBOOL("bhop"));
                            ENDGROUPBOX();
                            break;
                        }
                        case 1: { /* skins */
                            BEGINGROUPBOX("skin changer", ImVec2(438, 214));
                                static int curWeaponSelected = 0;
                                COMBOBOX("weapon", &curWeaponSelected, SkinChanger::weapons, IM_ARRAYSIZE(SkinChanger::weapons));
                                if (curWeaponSelected != 0) {
                                    const char* curWeaponName = SkinChanger::weapons[curWeaponSelected];
                                    char buf[256] = "skin changer ";
                                    strcat(buf, curWeaponName);

                                    char buf2[256];
                                    strcpy(buf2, buf);
                                    strcat(buf2, " paintkit");
                                    int temp = CONFIGFLOAT(buf2);
                                    SLIDERINT("paintkit ID", &temp, 0, 1000, "%d");
                                    CONFIGFLOAT(buf2) = temp;

                                    strcpy(buf2, buf);
                                    strcat(buf2, " wear");
                                    SLIDERFLOAT("wear", &CONFIGFLOAT(buf2), 0, 1, "%.3f");

                                    strcpy(buf2, buf);
                                    strcat(buf2, " stattrack");
                                    temp = CONFIGFLOAT(buf2);
                                    SLIDERINT("stattrack", &temp, 0, 999999, "%d");
                                    CONFIGFLOAT(buf2) = temp;

                                }
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(6, 262));

                            BEGINGROUPBOX("model changer", ImVec2(438, 131));
                                int temp = CONFIGINT("knife model");
                                COMBOBOX("knife model", &temp, SkinChanger::knives, IM_ARRAYSIZE(SkinChanger::knives));
                                CONFIGFLOAT("knife model") = temp;
                            ENDGROUPBOX();
                            break;
                        }
                    }
                    break;
                }
                case 3: {
                    ImGui::SetCursorPos(ImVec2(6, 6));

                    BEGINGROUPBOX("configs", ImVec2(166, 163));

                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    if (ImGui::BeginListBox("configlist", ImVec2(0, 70))) {
                        ImGui::SetCursorPosY(6);
                        for (std::string file : Config::cfgFiles) {
                            if (ImGui::Button(file.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 16))) {
                                strcpy(Config::selectedCfg, file.c_str());
                            }
                        }
                        ImGui::EndListBox();
                    }
                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    ImGui::InputText("##cfgname", Config::selectedCfg, sizeof(Config::selectedCfg));
                    if (ImGui::Button("save", ImVec2((ImGui::GetWindowContentRegionWidth() - 16) / 3, 16))) {
                        strcpy(curConfigLoaded, Config::selectedCfg);
                        Config::saveConfig(Config::selectedCfg);
                        Config::refreshConfigList();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("load", ImVec2((ImGui::GetWindowContentRegionWidth() - 16) / 3, 16))) {
                        strcpy(curConfigLoaded, Config::selectedCfg);
                        Config::loadConfig(Config::selectedCfg);
                        Config::refreshConfigList();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("delete", ImVec2((ImGui::GetWindowContentRegionWidth() - 16) / 3, 16))) {
                        char path[512];
                        strcpy(path, getenv("HOME"));
                        strcat(path, "/.eclipse/");
                        std::filesystem::create_directory(path);
                        strcat(path, "configs/");
                        std::filesystem::create_directory(path);
                        strcat(path, Config::selectedCfg);

                        if (Config::selectedCfg[0] != '\0' && std::filesystem::exists(path))
                            std::filesystem::remove(path);
                        Config::refreshConfigList();
                    }
                    if (ImGui::Button("refresh", ImVec2(ImGui::GetWindowContentRegionWidth(), 16))) {
                        Config::refreshConfigList();
                    }

                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(6, 173));

                    BEGINGROUPBOX("scripts", ImVec2(166, 220));
                    
                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                    if (ImGui::BeginListBox("configlist", ImVec2(0, 164))) {
                        ImGui::SetCursorPosY(6);
                        for (std::string file : Lua::luaFiles) {
                            char temp[128] = "luafiles - ";
                            strcat(temp, file.c_str());
                            strcat(temp, "enabled");
                            ImGui::Checkbox(file.c_str(), &CONFIGBOOL(temp));
                            if (ImGui::IsItemHovered() ) {
                                char path[512];
                                strcpy(path, getenv("HOME"));
                                strcat(path, "/.eclipse/");
                                std::filesystem::create_directory(path);
                                strcat(path, "scripts/");
                                std::filesystem::create_directory(path);
                                strcat(path, file.c_str());

                                std::ifstream infile(path);
                                if (infile.good()) {
                                    std::string line;
                                    getline(infile, line);
                                    if (line.rfind("--name ", 0) == 0) {
                                        ImGui::BeginTooltip();
                                        ImGui::PushTextWrapPos(280);
                                        ImGui::Text("%s", line.substr(7).c_str());
                                        getline(infile, line);
                                        if (line.rfind("--desc ", 0) == 0) {
                                            ImGui::Separator();
                                            ImGui::Text("%s", line.substr(7).c_str());
                                            getline(infile, line);
                                            if (line.rfind("--author ", 0) == 0) {
                                                ImGui::Text(" - %s", line.substr(9).c_str());
                                            }
                                        }
                                        ImGui::PopTextWrapPos();
                                        ImGui::EndTooltip();
                                    }
                                }
                                infile.close();

                            }
                        }
                        ImGui::EndListBox();
                    }
                    if (ImGui::Button("refresh", ImVec2(ImGui::GetWindowContentRegionWidth(), 16))) {
                        Lua::refreshLuaList();
                    }

                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(178, 6));

                    BEGINGROUPBOX("script UI", ImVec2(266, 387));
                    Lua::handleHook("UI");

                    ENDGROUPBOX();
                    break;
                }
            }

            ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("tabs", ImVec2(ImGui::GetWindowContentRegionWidth(), 30), true);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 0.8, 0.8));
            static int lastCurTab = 0;
            if (lastCurTab == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            if (ImGui::Button("legit", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) curTab = 0;
            if (lastCurTab == 0) ImGui::PopStyleColor();
            ImGui::SameLine();
            if (lastCurTab == 1) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            if (ImGui::Button("visuals", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) curTab = 1;
            if (lastCurTab == 1) ImGui::PopStyleColor();
            ImGui::SameLine();
            if (lastCurTab == 2) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            if (ImGui::Button("misc", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) curTab = 2;
            if (lastCurTab == 2) ImGui::PopStyleColor();
            ImGui::SameLine();
            if (lastCurTab == 3) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            if (ImGui::Button("config/scripts", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) curTab = 3;
            if (lastCurTab == 3) ImGui::PopStyleColor();
            lastCurTab = curTab;
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::End();
        }
        else {
            ImGui::GetIO().MouseDrawCursor = false;
        }

        Lua::curDrawList = ImGui::GetForegroundDrawList();
        Lua::handleHook("drawabove");

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        for (std::string file : Lua::luaFiles) {
            char temp[128] = "luafiles - ";
            strcat(temp, file.c_str());
            strcat(temp, "enabled");
            if (Lua::scripts.find(file) != Lua::scripts.end()) {
                if (!CONFIGBOOL(temp)) {
                    if (Lua::scripts.at(file).hooks.find("unload") != Lua::scripts.at(file).hooks.end()) {
                        try {
                            Lua::scripts.at(file).hooks.at("unload")();
                        }
                        catch (luabridge::LuaException const& e) {
                            ERR("lua error (%s): %s", file.c_str(), e.what());
                        }
                    }
                    Lua::scripts.erase(file);
                }
            }
            else {
                if (CONFIGBOOL(temp)) {
                    Lua::scripts.insert(std::pair<std::string, Lua::LuaEngine>(file, Lua::LuaEngine(file)));
                }
            }
        }
    }
}