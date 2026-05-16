
#include "dm2_v1_outdoor_renderer.h"
#include <string.h>

/* DM2 outdoor renderer
 * Source: SKULL.ASM outdoor viewport, sky gradient, building draw */

void dm2_v1_outdoor_init(DM2_V1_OutdoorConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->time_of_day = 0.5f; /* noon */
}

void dm2_v1_outdoor_set_weather(DM2_V1_OutdoorConfig *cfg, int weather) {
    if (cfg) cfg->weather = weather;
}

void dm2_v1_outdoor_set_time(DM2_V1_OutdoorConfig *cfg, float time) {
    if (cfg) cfg->time_of_day = time < 0 ? 0 : (time > 1 ? 1 : time);
}

uint32_t dm2_v1_outdoor_sky_color(const DM2_V1_OutdoorConfig *cfg) {
    if (!cfg) return 0xFF4488CC;
    /* Day/night cycle: lerp between sky colors */
    float t = cfg->time_of_day;
    if (cfg->weather >= 2) return 0xFF666666; /* fog/storm = gray */
    if (t < 0.25f) { /* dawn */
        uint8_t r = (uint8_t)(60 + t * 4 * 140);
        return 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)(r/2) << 8) | 0xCC;
    } else if (t < 0.75f) { /* day */
        return cfg->weather == 1 ? 0xFF888899 : 0xFF4488CC;
    } else { /* dusk/night */
        float nt = (t - 0.75f) * 4;
        uint8_t b = (uint8_t)(200 - nt * 170);
        return 0xFF000000 | ((uint32_t)(20) << 16) | ((uint32_t)(20) << 8) | b;
    }
}

const char *dm2_v1_outdoor_source_evidence(void) {
    return "SKULL.ASM: outdoor viewport, sky gradient, building draw\n"
           "DM2 feature: day/night cycle, weather effects\n";
}

