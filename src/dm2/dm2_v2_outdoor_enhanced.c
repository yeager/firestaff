
#include "dm2_v2_outdoor_enhanced.h"
#include <string.h>

void dm2_v2_outdoor_fx_init(DM2_V2_OutdoorFX *fx) {
    if (fx) memset(fx, 0, sizeof(*fx));
}

void dm2_v2_outdoor_fx_tick(DM2_V2_OutdoorFX *fx, float dt, int weather) {
    if (!fx) return;
    fx->cloud_offset += dt * 5.0f;
    fx->tree_sway_phase += dt * 2.0f;
    if (fx->tree_sway_phase > 6.28f) fx->tree_sway_phase -= 6.28f;

    switch (weather) {
        case 1: fx->rain_intensity = 0.5f; fx->wind_strength = 0.3f; break;
        case 3: fx->rain_intensity = 1.0f; fx->wind_strength = 0.8f;
            fx->lightning_timer -= dt;
            if (fx->lightning_timer <= 0) {
                fx->lightning_flash = 1;
                fx->lightning_timer = 3.0f + (float)((int)(dt * 1000) % 5);
            } else {
                fx->lightning_flash = 0;
            }
            break;
        default: fx->rain_intensity = 0; fx->wind_strength = 0; break;
    }
}

void dm2_v2_outdoor_fx_trigger_lightning(DM2_V2_OutdoorFX *fx) {
    if (fx) { fx->lightning_flash = 1; fx->lightning_timer = 0.1f; }
}

const char *dm2_v2_outdoor_source_evidence(void) {
    return "DM2 V2.2: animated weather (rain particles, lightning flash)\n"
           "DM2 V2.2: cloud movement, tree sway animation\n";
}

