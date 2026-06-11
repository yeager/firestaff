#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L_D3R_SIDEWALL_BACKDROPS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L_D3R_SIDEWALL_BACKDROPS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB V1 D3L/D3R side-wall composition gate.
 * ReDMCSB anchors: DUNVIEW.C F0116 lines 6361-6480, F0117 lines
 * 6500-6622, F0107 lines 3502-3938, F0108 lines 3940-4009,
 * F0115 lines 4547-4581, F0128 lines 8478-8500, F0104 lines
 * 3113-3156, F0105 lines 3185-3247; DEFS.H lines 2608-2609,
 * 2668-2677, 2698-2702, 2752-2754, and 4045-4046. CSB-lineage
 * anchors: Viewport.cpp lines 1192-1209 and 1903-1915.
 */

typedef enum {
    CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3L_PC34 = 0,
    CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3R_PC34 = 1
} CSB_V1_D3LD3RSidewallSidePc34;

typedef enum {
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_WALL_PC34 = 0,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_CORRIDOR_PC34 = 1,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_PIT_PC34 = 2,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_TELEPORTER_PC34 = 5,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_SIDE_PC34 = 16,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_FRONT_PC34 = 17,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_STAIRS_SIDE_PC34 = 18,
    CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_STAIRS_FRONT_PC34 = 19
} CSB_V1_D3LD3RSidewallElementPc34;

typedef struct {
    int side;
    const char *name;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int f0128_order_after_d3l2_d3r2;
    int preceding_backdrop_count;
    int wall_zone;
    int native_wall_index;
    int flipped_wall_index;
    int wall_frame_x1;
    int wall_frame_x2;
    int wall_frame_y1;
    int wall_frame_y2;
    int wall_frame_byte_width;
    int wall_frame_height;
    int side_wall_ornament_view;
    int front_wall_ornament_view;
    int floor_ornament_view;
    unsigned int open_order;
    unsigned int door_side_order;
    unsigned int door_rear_order;
    unsigned int door_front_order;
    int c10_transparent_color;
    int csb_lineage_open_contents_order;
    int csb_lineage_door_rear_order;
    int csb_lineage_door_front_order;
    const char *redmcsb_anchor;
} CSB_V1_D3LD3RSidewallBackdropSpecPc34;

typedef struct {
    int ok;
    int preceding_backdrops;
    int d3l_before_d3r;
    int wall_blit_calls;
    int f0104_calls;
    int f0105_calls;
    int f0107_calls;
    int f0108_calls;
    int f0111_calls;
    int f0115_calls;
    unsigned int first_f0115_order;
    unsigned int second_f0115_order;
    int wall_returns_without_front_alcove;
    int front_alcove_uses_f0115_zero_order;
    int c10_transparency_preserved;
} CSB_V1_D3LD3RSidewallBackdropTracePc34;

size_t csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_count_pc34(void);

const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *
csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(size_t index);

const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *
csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_for_side_pc34(int side);

int csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *spec,
    int element,
    int front_wall_ornament_is_alcove,
    CSB_V1_D3LD3RSidewallBackdropTracePc34 *out_trace);

uint8_t csb_v1_viewport_d3l_d3r_sidewall_backdrops_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *csb_v1_viewport_d3l_d3r_sidewall_backdrops_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
