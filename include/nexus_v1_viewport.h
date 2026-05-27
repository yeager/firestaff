
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
} Nexus_Viewport;

void nexus_viewport_init(Nexus_Viewport *vp);

/* Render one frame */
void nexus_viewport_render(Nexus_Viewport *vp, Nexus_V1_Engine *engine);

/* Convert indexed framebuffer to RGBA for SDL presentation */
void nexus_viewport_to_rgba(const Nexus_Viewport *vp, uint32_t *rgba_out);

/* Full render frame — orchestrates all rendering layers */
#include "nexus_v1_rendering.h"
void nexus_v1_render_frame(Nexus_Framebuffer *fb,
    Nexus_Camera *cam,
    Nexus_V1_Engine *engine,
    const Nexus_Projectile *projectiles, int projectile_count,
    const Nexus_V1_CreatureManager *creatures,
    int frame);

#endif

