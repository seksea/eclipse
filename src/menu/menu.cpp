#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_freetype.h"
#include "imgui/GL/gl3w.h"
#include "menu.hpp"
#include "fonts.hpp"

#define BEGINGROUPBOX(name, size) ImGui::BeginChild(name, size, true); ImGui::Text(name); ImGui::Separator()
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

            ImVec4* colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_WindowBg]               = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_Border]                 = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);

            ImGui_ImplOpenGL3_Init("#version 100");
            ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
            initialised = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);

        if (ImGui::IsKeyPressed(SDL_SCANCODE_INSERT, false)) {
            menuOpen = !menuOpen;
        }

        if (menuOpen) {
            ImGui::GetIO().MouseDrawCursor = true;
            ImGui::Begin("csgo-cheat", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 1), ImColor(0.09f, 0.09f, 0.09f));

            ImGui::BeginChild("content", ImVec2(450, 400), true);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.16f, 1.00f));
            BEGINGROUPBOX("aimbot", ImVec2(438, 260));
            static bool pog = false;
            CHECKBOX("enabled", &pog);
            ENDGROUPBOX();

            ImGui::SetCursorPos(ImVec2(6, 272));

            BEGINGROUPBOX("other", ImVec2(216, 121));
            
            ENDGROUPBOX();

            ImGui::SetCursorPos(ImVec2(228, 272));

            BEGINGROUPBOX("triggerbot", ImVec2(216, 121));
            
            ENDGROUPBOX();

            ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("tabs", ImVec2(ImGui::GetWindowContentRegionWidth(), 30), true);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            if (ImGui::Button("legit", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) { curTab = 0; }
            ImGui::SameLine();
            if (ImGui::Button("visuals", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) { curTab = 1; }
            ImGui::SameLine();
            if (ImGui::Button("misc", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) { curTab = 2; }
            ImGui::SameLine();
            if (ImGui::Button("config/scripts", ImVec2(ImGui::GetWindowWidth()/4, ImGui::GetWindowHeight()))) { curTab = 3; }
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

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}