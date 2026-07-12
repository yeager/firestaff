
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
    int no_draw_structure2_source;
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
    int floor_material_surface_count;
    int ceiling_material_surface_count;
    int wall_material_surface_count;
    int bpk_material_surface_count;
    int bpk_floor_material_surface_count;
    int bpk_ceiling_material_surface_count;
    int bpk_wall_material_surface_count;
    int rasterized_command_count;
    int palette_synced;
    int written_pixels;
    int captured_frame_ready;
    uint32_t frame_hash;
} Nexus_V1_DgnViewportRenderReceipt;

typedef enum {
    NEXUS_V1_DGN_HOST_ROUTE_MISSING = 0,
    NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH = 1,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_HANDOFF = 2,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_VIEWPORT_NOT_RENDERED = 3,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_MATERIALS = 4,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_RASTER = 5,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE2_SOURCE = 6
} Nexus_V1_DgnViewportHostRouteStatus;

typedef struct {
    Nexus_V1_DgnViewportHostRouteStatus status;
    Nexus_V1_DgnRendererHandoffStatus handoff_status;
    int package_consumed;
    int host_route_consumed;
    int can_present_runtime_dgn;
    int blocks_runtime_dgn;
    int fallback_visuals_permitted;
    int no_draw_structure2_source;
    int level;
    int party_x;
    int party_y;
    int party_dir;
    int command_count;
    int floor_count;
    int ceiling_count;
    int wall_count;
    int material_surface_count;
    int bpk_material_surface_count;
    int bpk_floor_material_surface_count;
    int bpk_ceiling_material_surface_count;
    int bpk_wall_material_surface_count;
    int rasterized_command_count;
    int written_pixels;
    int palette_synced;
    int captured_frame_ready;
    uint32_t frame_hash;
    int missing_material_count;
    int first_missing_material_id;
    Nexus_V1_DgnRenderCommandKind first_missing_material_kind;
    int post_grid_0x30_ref_unique_count;
    int max_post_grid_0x30_ref;
} Nexus_V1_DgnViewportHostRouteReceipt;

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

int nexus_viewport_dgn_host_route_receipt(
    const Nexus_Viewport *vp,
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnViewportHostRouteReceipt *out_receipt);
const char *nexus_viewport_dgn_host_route_status_name(
    Nexus_V1_DgnViewportHostRouteStatus status);

#endif
