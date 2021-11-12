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
            ImGui_ImplOpenGL3_Init("#version 100");
            ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
            initialised = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);

        ImGui::NewFrame();
        
        ImGui::Text("ahhhhhhhhh");

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}