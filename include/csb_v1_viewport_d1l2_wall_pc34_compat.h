#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} CSB_V1_ViewportD1L2WallBlitStatsPc34;

typedef struct {
    int source_locked_contract_only;
    int no_asset_parity;
    int requested_d1l2_addressable;
    int uses_d1l_closest_analogue;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int wall_element;
    int teleporter_element;
    int wall_zone;
    int neighboring_d1c_zone;
    int neighboring_d1r_zone;
    int native_wall_index;
    int flipped_wall_index;
    int frame_array_index;
    int frame_x1;
    int frame_x2;
    int frame_y1;
    int frame_y2;
    int frame_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
    int clip_width;
    int clip_height;
    int transparent_color;
    int f0100_st_wall_route;
    int f0104_i34_wall_route;
    int f0105_i34_flipped_route;
    int f0107_wall_ornament_route;
    int f0111_door_route;
    int f0113_teleporter_route;
    int f0115_wall_thing_pass_route;
    const char *bitmap_symbol;
    const char *frame_symbol;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD1L2WallRouteSpecPc34;

const CSB_V1_ViewportD1L2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_wall_route_spec_pc34(void);

int csb_v1_viewport_d1l2_wall_resolve_clip_pc34(
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec,
    int *out_x,
    int *out_y,
    int *out_width,
    int *out_height);

int csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int flip_horizontal,
    CSB_V1_ViewportD1L2WallBlitStatsPc34 *stats);

const char *csb_v1_viewport_d1l2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H */
