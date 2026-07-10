
#ifndef NEXUS_V1_VIEWPORT_H
#define NEXUS_V1_VIEWPORT_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_engine.h"

/* Render the 3D dungeon viewport from party position.
 * View distance: 4 squares ahead (like DM1 D0-D3).
 * Renders walls, floor, ceiling, and creatures in view. */

#define NEXUS_VIEW_DISTANCE 4

typedef struct {
    Nexus_Framebuffer fb;
    Nexus_Camera cam;
    int render_wireframe;  /* debug mode */
    const Nexus_V1_Engine *material_engine;
    uint32_t material_generation;
    uint32_t base_palette[256];
    uint8_t floor_material_palette_map[NEXUS_DMDF_MATERIAL_COUNT][256];
    uint8_t wall_material_palette_map[NEXUS_DMDF_MATERIAL_COUNT][256];
    int material_palette_valid;
} Nexus_Viewport;

void nexus_viewport_init(Nexus_Viewport *vp);

/* Render one frame */
void nexus_viewport_render(Nexus_Viewport *vp, Nexus_V1_Engine *engine);

/* Convert indexed framebuffer to RGBA for SDL presentation */
void nexus_viewport_to_rgba(const Nexus_Viewport *vp, uint32_t *rgba_out);

#endif
