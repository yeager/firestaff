
#ifndef FIRESTAFF_DM2_V1_OUTDOOR_RENDERER_H
#define FIRESTAFF_DM2_V1_OUTDOOR_RENDERER_H
#include <stdint.h>

/* DM2 Outdoor Renderer — sky, trees, buildings
 * DM2's signature feature: outdoor exploration with sky and buildings.
 * Indoor areas use the DM1-style first-person renderer.
 * Source: SKULL.ASM outdoor viewport routines */

typedef struct {
    int sky_texture;
    int ground_texture;
    int tree_density;
    int building_count;
    int weather; /* 0=clear, 1=rain, 2=fog, 3=storm */
    float time_of_day; /* 0.0-1.0, affects sky color */
} DM2_V1_OutdoorConfig;

void dm2_v1_outdoor_init(DM2_V1_OutdoorConfig *cfg);
void dm2_v1_outdoor_set_weather(DM2_V1_OutdoorConfig *cfg, int weather);
void dm2_v1_outdoor_set_time(DM2_V1_OutdoorConfig *cfg, float time);
uint32_t dm2_v1_outdoor_sky_color(const DM2_V1_OutdoorConfig *cfg);
const char *dm2_v1_outdoor_source_evidence(void);
#endif

