#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_VIEWPORT_D3L2_D3R2_WALL_SIDE_LEFT_PC34 = 0,
    CSB_V1_VIEWPORT_D3L2_D3R2_WALL_SIDE_RIGHT_PC34 = 1
} CSB_V1_ViewportD3L2D3R2WallSidePc34;

typedef struct {
    int map_x;
    int map_y;
    int direction;
} CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34;

typedef struct {
    int map_x;
    int map_y;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int square_type;
} CSB_V1_ViewportD3L2D3R2WallPositionPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int left_drawn;
    int right_drawn;
    int draw_order_left_then_right;
    int relative_square_gate_ok;
    int depth3_attenuation_ok;
    int wall_band_clip_ok;
    int ornament_route_ok;
    int lighting_route_ok;
    int door_route_suppressed_for_wall_ok;
    int thing_pass_suppressed_for_wall_ok;
    int left_copied_pixels;
    int right_copied_pixels;
    int left_zone;
    int right_zone;
} CSB_V1_ViewportD3L2D3R2WallRenderResultPc34;

size_t csb_v1_viewport_d3l2_d3r2_wall_route_spec_count_pc34(void);

int csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
    const CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 *party,
    int relative_depth,
    int relative_lateral,
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 *out_position);

int csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(
    const CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 *party,
    const CSB_V1_ViewportD3L2D3R2WallPositionPc34 *left_wall,
    const CSB_V1_ViewportD3L2D3R2WallPositionPc34 *right_wall,
    const uint8_t *left_source,
    const uint8_t *right_source,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 *out_result);

const char *csb_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
