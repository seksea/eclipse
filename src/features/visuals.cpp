#include "visuals.hpp"
#include "../sdk/entity.hpp"
#include "../menu/config.hpp"
#include "../menu/menu.hpp"
#include "../util/protection/protection.hpp"

namespace Visuals {
    void nightmode(Entity* tonemapController) {
        if (CONFIGFLOAT("nightmode") == 0.f)
            return;
        tonemapController->nDT_EnvTonemapController__m_bUseCustomAutoExposureMin() = CONFIGFLOAT("nightmode") > 0.f ? 1 : 0;
        tonemapController->nDT_EnvTonemapController__m_bUseCustomAutoExposureMax() = CONFIGFLOAT("nightmode") > 0.f ? 1 : 0;

        tonemapController->nDT_EnvTonemapController__m_flCustomAutoExposureMin() = 1.01f - CONFIGFLOAT("nightmode");
        tonemapController->nDT_EnvTonemapController__m_flCustomAutoExposureMax() = 1.01f - CONFIGFLOAT("nightmode");
    }

    void skyboxChanger() {
        // remove 3d sky
        static Convar* r_3dsky = nullptr;
        if (CONFIGBOOL("remove 3d skybox")) {
            if (!r_3dsky)
                r_3dsky = Interfaces::cvar->findVar("r_3dsky");
            else
                r_3dsky->setInt(0);
        }
        else if (r_3dsky) {
            r_3dsky->setInt(1);
        }

        typedef bool (*SetNamedSkybox)(const char *);
        static SetNamedSkybox setNamedSkybox = (SetNamedSkybox)Memory::patternScan("engine_client.so", "55 4C 8D 05 ? ? ? ? 48 89 E5 41");

        if (setNamedSkybox && CONFIGINT("skybox")) {
            setNamedSkybox((CONFIGINT("skybox") != 1) ? skyboxes[CONFIGINT("skybox")] : "sky_l4d_rural02_ldr");
        }
    }

    void watermark() {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.16f, 1.00f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(5, 5), ImVec2(600, 25));
        ImGui::SetNextWindowPos(ImVec2(5,5));
        ImGui::Begin("watermark", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | (Menu::menuOpen ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs));
        ImGui::Text("eclipse.wtf %s | %.1fFPS", Protection::username, ImGui::GetIO().Framerate);
        ImGui::End();
        ImGui::PopStyleColor();
    }
}