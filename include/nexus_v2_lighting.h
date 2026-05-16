
#ifndef NEXUS_V2_LIGHTING_H
#define NEXUS_V2_LIGHTING_H
#include <stdint.h>

/* Dynamic lighting for Nexus V2.2.
 * - Torch held by party: warm radius, flicker
 * - Spell cast: flash + color based on element
 * - Ambient: dungeon-level base light
 * - Distance attenuation from light source */

typedef struct {
    float x, y, z;
    float r, g, b;
    float intensity;
    float radius;
    int flicker;       /* 0=steady, 1=flicker */
    int active;
} Nexus_Light;

#define NEXUS_MAX_LIGHTS 16

typedef struct {
    Nexus_Light lights[NEXUS_MAX_LIGHTS];
    int light_count;
    float ambient_r, ambient_g, ambient_b;
    float torch_flicker_phase;
} Nexus_V2_LightingState;

void nexus_v2_lighting_init(Nexus_V2_LightingState *ls);
void nexus_v2_lighting_tick(Nexus_V2_LightingState *ls, float dt);
int nexus_v2_light_add(Nexus_V2_LightingState *ls, float x, float y, float z,
    float r, float g, float b, float intensity, float radius, int flicker);
void nexus_v2_light_remove(Nexus_V2_LightingState *ls, int index);

/* Apply lighting to RGBA framebuffer */
void nexus_v2_apply_lighting(uint32_t *rgba, int w, int h,
    const Nexus_V2_LightingState *ls,
    float cam_x, float cam_y, float cam_z);

#endif

