#pragma once
#include "../interfaces.hpp"

namespace Visuals {
    inline const char* skyboxes[] = {
            "None",
            "Black", // sky_l4d_rural02_ldr
            "cs_baggage_skybox_",
            "cs_tibet",
            "embassy",
            "italy",
            "jungle",
            "nukeblank",
            "office",
            "sky_cs15_daylight01_hdr",
            "sky_cs15_daylight02_hdr",
            "sky_cs15_daylight03_hdr",
            "sky_cs15_daylight04_hdr",
            "sky_csgo_cloudy01",
            "sky_csgo_night_flat",
            "sky_csgo_night02",
            "sky_csgo_night02b",
            "sky_day02_05",
            "sky_day02_05_hdr",
            "sky_dust",
            "sky_hr_aztec",
            "sky_lunacy",
            "sky_venice",
            "vertigo",
            "vertigo_hdr",
            "vertigoblue_hdr",
            "vietnam"
    };

    void nightmode(Entity* tonemapController);
    void skyboxChanger();
    void watermark();
}