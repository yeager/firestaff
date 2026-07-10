
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
    int attempted;
    int ready;
    int used_real_dgn_route;
    int blocked;
    int fallback_visuals_permitted;
    int party_x;
    int party_y;
    int party_dir;
    int command_count;
    int floor_count;
    int ceiling_count;
    int wall_count;
    int missing_material_count;
    int first_missing_material_id;
    Nexus_V1_DgnRenderCommandKind first_missing_material_kind;
    int material_surface_count;
    int rasterized_command_count;
    int palette_synced;
    int written_pixels;
} Nexus_V1_DgnViewportRenderReceipt;

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
    Nexus_V1_DgnViewportRenderReceipt last_dgn_render_receipt;
} Nexus_Viewport;

void nexus_viewport_init(Nexus_Viewport *vp);

/* Render one frame */
void nexus_viewport_render(Nexus_Viewport *vp, Nexus_V1_Engine *engine);

/* Convert indexed framebuffer to RGBA for SDL presentation */
void nexus_viewport_to_rgba(const Nexus_Viewport *vp, uint32_t *rgba_out);

int nexus_viewport_last_dgn_render_receipt(
    const Nexus_Viewport *vp,
    Nexus_V1_DgnViewportRenderReceipt *out_receipt);

#endif
