#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_WALL_PC34_COMPAT_H

#include <stdint.h>

typedef enum {
    CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2 = 0,
    CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2 = 1
} CSB_V1_ViewportD3L2WallSide;

typedef struct {
    int view_square;
    int wall_zone;
    int zone_record_type;
    int zone_parent_record;
    int viewport_clip_record;
    int coord_layout_range;
    int csb_exetype_c03;
    int native_wall_index;
    int opposite_wall_index;
    int frame_x1;
    int frame_x2;
    int frame_y1;
    int frame_y2;
    int byte_width;
    int height;
    int source_x;
    int source_y;
    int bitmap_command;
    int bitmap_index;
    int rect_command;
    int rect_index;
    int blit_command;
    int uses_f0105_scratch_flip;
} CSB_V1_ViewportD3L2WallSideSpec;

typedef struct {
    int transparent_color;
    int preserves_c10_transparency;
    int source_locked_contract_only;
    const char *redmcsb_function;
    const char *csb_source_function;
    const char *source_lines;
    CSB_V1_ViewportD3L2WallSideSpec d3l2;
    CSB_V1_ViewportD3L2WallSideSpec d3r2;
} CSB_V1_ViewportD3L2WallRouteSpec;

const CSB_V1_ViewportD3L2WallRouteSpec *
csb_v1_viewport_d3l2_wall_route_spec_pc34(void);

const CSB_V1_ViewportD3L2WallSideSpec *
csb_v1_viewport_d3l2_wall_side_spec_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side);

int csb_v1_viewport_d3l2_wall_resolve_zone_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side,
    int *out_x,
    int *out_y,
    int *out_width,
    int *out_height);

int csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int flip_horizontal);

const char *csb_v1_viewport_d3l2_wall_source_evidence_pc34(void);

#endif
