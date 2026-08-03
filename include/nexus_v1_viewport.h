
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
    int no_draw_structure1f_semantics;
    /* A complete source-bound Structure3 scene supersedes the older
     * Structure1B/MNS raster path until original Saturn material and VDP1
     * semantics are independently captured. */
    int no_draw_structure3_source_scene;
    int structure3_mesh_rendered;
    int structure3_mesh_face_count;
    int structure3_mesh_textured_face_count;
    int structure3_mesh_flat_color_face_count;
    int structure3_mesh_animated_static_fallback_face_count;
    int active_level_source_consumed;
    Nexus_V1_DgnActiveLevelRendererSourceReceipt active_level_source;
    int structure3_source_packet_consumed;
    int structure3_source_geometry_bound;
    int structure3_source_no_draw;
    int structure3_runtime_vdp1_command_framed;
    int structure3_runtime_vdp1_texture_format_framed;
    int structure3_runtime_vdp1_coordinate_words_framed;
    int structure3_runtime_vdp1_vram_window_bound;
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt
        structure3_runtime_vdp1_vram_window;
    int structure3_runtime_vdp1_command_vram_bound;
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt
        structure3_runtime_vdp1_command_vram;
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt
        structure3_runtime_vdp1_command;
    /* A package-owned Structure3 face/vertex/normal packet may be consumed
     * before any external Saturn capture exists. It is a no-draw source
     * staging lane, separate from the opaque capture packet above. */
    int structure3_package_geometry_consumed;
    int structure3_package_geometry_bound;
    int structure3_package_geometry_no_draw;
    int structure3_package_geometry_scene_consumed;
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt
        structure3_package_geometry_scene;
    int structure3_animated_material_scene_consumed;
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt
        structure3_animated_material_scene;
    int structure3_animated_material_image_scene_consumed;
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt
        structure3_animated_material_image_scene;
    int structure3_untextured_face_scene_consumed;
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt
        structure3_untextured_face_scene;
    int structure3_complete_source_scene_consumed;
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt
        structure3_complete_source_scene;
    int structure1f_source_scene_consumed;
    Nexus_V1_DgnStructure1FSourceSceneReceipt structure1f_source_scene;
    int structure1f2_face_adjacency_transform_consumed;
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt
        structure1f2_face_adjacency_transform;
    int m11_direct_lev_dungeon_no_draw_consumed;
    Nexus_V1_DgnM11DirectLevNoDrawReceipt m11_direct_lev_dungeon_no_draw;
    int structure1c_source_scene_consumed;
    Nexus_V1_DgnStructure1CSourceSceneReceipt structure1c_source_scene;
    int structure2_payload_anchor_scene_consumed;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt
        structure2_payload_anchor_scene;
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
    int static_mns_material_surface_count;
    int static_mns_floor_material_surface_count;
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
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE2_SOURCE = 6,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE1F_SEMANTICS = 7,
    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE3_SOURCE_SCENE = 8
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
    int no_draw_structure1f_semantics;
    int no_draw_structure3_source_scene;
    int active_level_source_consumed;
    Nexus_V1_DgnActiveLevelRendererSourceReceipt active_level_source;
    int level;
    int party_x;
    int party_y;
    int party_dir;
    int command_count;
    int floor_count;
    int ceiling_count;
    int wall_count;
    int material_surface_count;
    int static_mns_material_surface_count;
    int static_mns_floor_material_surface_count;
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

typedef struct Nexus_Viewport {
    Nexus_Framebuffer fb;
    Nexus_Camera cam;
    int render_wireframe;  /* debug mode */
    const Nexus_V1_Engine *material_engine;
    uint32_t material_generation;
    uint32_t base_palette[256];
    uint8_t floor_material_palette_map[NEXUS_DMDF_MATERIAL_COUNT][256];
    uint8_t wall_material_palette_map[NEXUS_DMDF_MATERIAL_COUNT][256];
    int material_palette_valid;
    Nexus_V1_DgnStructure3RenderPacket structure3_source_packet;
    /* First packet is retained as an inspection exemplar; the receipt records
     * traversal of the complete source-bound static-material scene. */
    Nexus_V1_DgnStructure3PackageGeometryPacket structure3_package_geometry;
    Nexus_V1_DgnStructure3AnimatedMaterialPacket structure3_animated_material;
    Nexus_V1_DgnStructure3AnimatedMaterialImagePacket
        structure3_animated_material_image;
    Nexus_V1_DgnStructure3UntexturedFacePacket structure3_untextured_face;
    Nexus_V1_DgnStructure1FSourcePacket structure1f_source_packet;
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt
        structure1f2_face_adjacency_transform;
    Nexus_V1_DgnStructure1CSourcePacket structure1c_source_packet;
    Nexus_V1_DgnStructure2PayloadAnchorPacket structure2_payload_anchor_packet;
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
