#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_freetype.h"
#include "imgui/GL/gl3w.h"
#include "menu.hpp"
#include "fonts.hpp"
#include "config.hpp"
#include "../features/lua.hpp"

#define BEGINGROUPBOX(name, size) ImGui::BeginChild(name, size, true); ImGui::TextColored(ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight())) ? ImVec4(1.f, 1.f, 1.f, 1.f) : ImVec4(0.8f, 0.8f, 0.8f, 1.f), name); ImGui::Separator()
#define ENDGROUPBOX() ImGui::EndChild()
#define CHECKBOX(name, var) ImGui::Checkbox(name, var)

namespace Menu {
    void onPollEvent(SDL_Event* event, const int result) {
        if (result && ImGui_ImplSDL2_ProcessEvent(event) && menuOpen) {
            event->type = 0;
        }
    }

    ImFont* menuFont;

    void onSwapWindow(SDL_Window* window) {
        if (!initialised) {
            gl3wInit();
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImFontConfig fontConfig;
            fontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_Monochrome;
            menuFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(tahoma_compressed_data, tahoma_compressed_size, 14, &fontConfig);
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
            ImGui::Begin("csgo-cheat", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            windowPos = ImGui::GetWindowPos();
            windowSize = ImGui::GetWindowSize();
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 1), ImColor(0.09f, 0.09f, 0.09f));

            ImGui::BeginChild("content", ImVec2(450, 400), true);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.16f, 1.00f));
            switch (curTab) {
                case 0: {
                    ImGui::SetCursorPos(ImVec2(6, 6));

                    BEGINGROUPBOX("aimbot", ImVec2(438, 250));
                    CHECKBOX("enabled", &CONFIGBOOL("aimbot enabled"));
                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(6, 262));

                    BEGINGROUPBOX("other", ImVec2(216, 131));
                    
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

                    switch (curSubTab) {
                        case 0: {
                            ImGui::SetCursorPos(ImVec2(6, 42));

                            BEGINGROUPBOX("ESP", ImVec2(216, 351));
                            
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 42));

                            BEGINGROUPBOX("chams", ImVec2(216, 351));
                            
                            ENDGROUPBOX();
                            break;
                        }
                        case 1: {
                            ImGui::SetCursorPos(ImVec2(6, 42));

                            BEGINGROUPBOX("ESP", ImVec2(216, 351));
                            
                            ENDGROUPBOX();

                            ImGui::SetCursorPos(ImVec2(228, 42));

                            BEGINGROUPBOX("chams", ImVec2(216, 351));
                            
                            ENDGROUPBOX();
                            break;
                        }
                        case 2: {
                            ImGui::SetCursorPos(ImVec2(6, 42));

                            BEGINGROUPBOX("world", ImVec2(216, 351));
                            
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

                    BEGINGROUPBOX("misc", ImVec2(438, 250));
                    
                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(6, 262));

                    BEGINGROUPBOX("clantag", ImVec2(216, 131));
                    
                    ENDGROUPBOX();

                    ImGui::SetCursorPos(ImVec2(228, 262));

                    BEGINGROUPBOX("movement", ImVec2(216, 131));
                    
                    ENDGROUPBOX();
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
                        Config::saveConfig(Config::selectedCfg);
                        Config::refreshConfigList();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("load", ImVec2((ImGui::GetWindowContentRegionWidth() - 16) / 3, 16))) {
                        Config::loadConfig(Config::selectedCfg);
                        Config::refreshConfigList();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("delete", ImVec2((ImGui::GetWindowContentRegionWidth() - 16) / 3, 16))) {
                        char path[512];
                        strcpy(path, getenv("HOME"));
                        strcat(path, "/.csgo-cheat/");
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
                            if (Lua::scripts.find(file) != Lua::scripts.end()) {
                                if (!CONFIGBOOL(temp)) {
                                    Lua::scripts.erase(file);
                                }
                            }
                            else {
                                if (CONFIGBOOL(temp)) {
                                    Lua::scripts.insert(std::pair<std::string, Lua::LuaEngine>(file, Lua::LuaEngine(file)));
                                }
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
        
        ImGui::ShowDemoWindow();

        Lua::curDrawList = ImGui::GetForegroundDrawList();
        Lua::handleHook("drawabove");

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}