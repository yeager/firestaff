#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int source_locked_contract_only;
    int view_square;
    int f0128_draw_order_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int wall_zone;
    int wall_element;
    int teleporter_element;
    int native_wall_index_base;
    int native_wall_index_pc_fix_delta;
    int native_wall_index_pc34_effective;
    int media709_flipped_wall_index;
    int f0104_wall_route;
    int f0105_media709_flipped_route;
    int f0113_teleporter_route;
    int f0111_door_route;
    int f0115_thing_pass_route;
    int transparent_color;
    int preserves_c10_transparency;
    int frame_blit_command_60200;
    int frame_rect_command_60250;
    int csb_viewport_wall_bitmap_index;
    int csb_viewport_wall_rectangle_index;
    int teleporter_field_aspect_index;
    const char *route_name;
    const char *redmcsb_function;
    const char *csb_lineage_anchor;
    const char *source_lines;
} CSB_V1_ViewportD2L2D2R2WallRouteSpec;

typedef struct {
    int ok;
    int route_count;
    int wall_zone_draw_order_ok;
    int palette_indices_ok;
    int lineage_frame_bindings_ok;
    int symmetry_ok;
    int d2l2_copied_pixels;
    int d2r2_copied_pixels;
} CSB_V1_ViewportD2L2D2R2WallRunResult;

size_t csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34(void);

const CSB_V1_ViewportD2L2D2R2WallRouteSpec *
csb_v1_viewport_d2l2_d2r2_wall_route_spec_at_pc34(size_t index);

const CSB_V1_ViewportD2L2D2R2WallRouteSpec *
csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(int view_square);

int csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal);

int csb_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(
    CSB_V1_ViewportD2L2D2R2WallRunResult *out_result);

const char *csb_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
