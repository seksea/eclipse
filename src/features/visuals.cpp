#include "visuals.hpp"
#include "../sdk/entity.hpp"
#include "../menu/config.hpp"

namespace Visuals {
    void nightmode(Entity* tonemapController) {
        tonemapController->nDT_EnvTonemapController__m_bUseCustomAutoExposureMin() = CONFIGFLOAT("nightmode") > 0.f ? 1 : 0;
        tonemapController->nDT_EnvTonemapController__m_bUseCustomAutoExposureMax() = CONFIGFLOAT("nightmode") > 0.f ? 1 : 0;

        tonemapController->nDT_EnvTonemapController__m_flCustomAutoExposureMin() = 1.01f - CONFIGFLOAT("nightmode");
        tonemapController->nDT_EnvTonemapController__m_flCustomAutoExposureMax() = 1.01f - CONFIGFLOAT("nightmode");
    }
}