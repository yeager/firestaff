#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_FRONT_CLIPPED_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_FRONT_CLIPPED_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C4000_HALF_ZONE_SHIFT 0x4000
#define CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C15_DESTROYED_MASK 15
#define CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C2_DOOR_ORNAMENT_D1LCR 2

typedef enum {
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2 = 0,
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2 = 1
} CSB_V1_D2L2D2R2F0111FrontClippedRoutePc34;

typedef enum {
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_NONE = 0,
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_CLOSED = 1,
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_HORIZONTAL = 2,
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_VERTICAL = 3,
    CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_DESTROYED = 4
} CSB_V1_D2L2D2R2F0111FrontClippedBranchPc34;

typedef struct {
    CSB_V1_D2L2D2R2F0111FrontClippedRoutePc34 route;
    int view_square;
    int draw_depth;
    int draw_lateral;
    int wall_zone;
    int front_clipped_half_is_right;
    int front_clipped_half_is_left;
    int door_zone_base;
    int door_ornament_view;
    int destroyed_mask_ornament;
    int transparent_color;
    int open_state;
    int closed_state;
    int destroyed_state;
    int horizontal_first_half_offset;
    int horizontal_final_half_offset;
    int horizontal_half_zone_shift_mask;
    int native_bitmap_width;
    int native_bitmap_height;
    int frame_clip_width;
    int frame_clip_height;
    int half_clip_width;
    int frame_x;
    int frame_y;
    int source_half_x;
    int zone_shift_x;
    int zone_shift_y;
    int pass1_order;
    int pass2_order;
    int frame_metadata_present;
    const char *route_name;
    const char *redmcsb_f0102_lines;
    const char *redmcsb_f0103_lines;
    const char *redmcsb_f0111_lines;
    const char *redmcsb_defs_lines;
    const char *lineage_source_lines;
} CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34;

typedef struct {
    int ok;
    CSB_V1_D2L2D2R2F0111FrontClippedBranchPc34 branch;
    int door_drawn;
    int pass_count;
    int ornament_view;
    int destroyed_mask_applied;
    int selected_bitmap_state;
    int uses_closed_or_destroyed_frame;
    int first_half_zone;
    int final_zone;
    int final_zone_without_shift_mask;
    int c4000_shift_applied;
    int half_zone_shift_x;
    int source_x;
    int source_width;
    int source_height;
    int destination_frame_x;
    int destination_frame_y;
    int pass1_order;
    int pass2_order;
    int transparent_color;
    const char *source_lock_evidence;
} CSB_V1_D2L2D2R2F0111FrontClippedTracePc34;

size_t csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_count_pc34(void);

const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_at_pc34(size_t index);

const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
    CSB_V1_D2L2D2R2F0111FrontClippedRoutePc34 route);

int csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *spec,
    int door_state,
    int horizontal_door,
    CSB_V1_D2L2D2R2F0111FrontClippedTracePc34 *out_trace);

int csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int destination_x,
    int height);

const char *csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
