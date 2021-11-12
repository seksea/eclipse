#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/GL/gl3w.h"
#include "menu.hpp"

namespace Menu {
    void onPollEvent(SDL_Event* event, const int result) {
        if (result && ImGui_ImplSDL2_ProcessEvent(event) && menuOpen) {
            event->type = 0;
        }
    }

    void onSwapWindow(SDL_Window* window) {
        if (!initialised) {
            gl3wInit();
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();


            ImGui::GetStyle().WindowPadding = ImVec2(3, 3);
            ImGui::GetStyle().WindowRounding = 0;
            ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);


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
            ImGui::Text("csgo-cheat");

            ImGui::BeginChild("tabs", ImVec2(80, 400), true);
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("content", ImVec2(500, 400), true);
            ImGui::EndChild();

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