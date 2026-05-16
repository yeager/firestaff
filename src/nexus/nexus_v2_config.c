
#include "nexus_v2_config.h"
#include <string.h>

void nexus_v2_config_init(Nexus_V2_Config *cfg, Nexus_V2_Mode mode) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->mode = mode;

    switch (mode) {
    case NEXUS_V2_UPSCALED:
        cfg->upscale_factor = 2;
        cfg->bilinear_filter = 1;
        cfg->widescreen = 0;
        cfg->render_width = 640;
        cfg->render_height = 400;
        break;
    case NEXUS_V2_ENHANCED:
        cfg->upscale_factor = 2;
        cfg->bilinear_filter = 1;
        cfg->widescreen = 1;
        cfg->render_width = 1280;
        cfg->render_height = 800;
        cfg->smooth_movement = 1;
        cfg->dynamic_lighting = 1;
        cfg->texture_filtering = 1;
        cfg->ambient_occlusion = 1;
        cfg->particles = 1;
        cfg->fog = 1;
        cfg->reflective_floors = 1;
        cfg->enhanced_creatures = 1;
        cfg->camera_bob = 1;
        cfg->screen_shake = 1;
        cfg->weather_fx = 1;
        cfg->enhanced_audio = 1;
        cfg->minimap = 1;
        cfg->damage_numbers = 1;
        cfg->journal = 1;
        cfg->achievements = 1;
        cfg->footstep_audio = 1;
        cfg->torch_flicker = 1;
        cfg->portrait_animations = 1;
        cfg->inventory_sort = 1;
        break;
    default: /* V1 Original */
        cfg->upscale_factor = 1;
        cfg->render_width = 320;
        cfg->render_height = 200;
        break;
    }
}

