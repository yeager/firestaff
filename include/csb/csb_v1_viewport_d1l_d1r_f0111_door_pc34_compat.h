#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_D1LR_STEP_FLOOR_ORNAMENT = 0,
    CSB_V1_D1LR_STEP_REAR_OBJECTS = 1,
    CSB_V1_D1LR_STEP_TOP_TRACK = 2,
    CSB_V1_D1LR_STEP_F0111_DOOR = 3,
    CSB_V1_D1LR_STEP_FRONT_OBJECTS = 4
} CSB_V1_ViewportD1LD1RF0111StepKind;

typedef struct {
    CSB_V1_ViewportD1LD1RF0111StepKind kind;
    int order;
    int zone;
    int view_square;
    int view_floor;
    int door_graphics;
    int door_rects;
    int transparent_color;
    int source_line;
    const char *name;
} CSB_V1_ViewportD1LD1RF0111Step;

typedef struct {
    int view_square;
    int depth;
    int lane;
    int map_forward;
    int map_side;
    int floor_view;
    int field_zone;
    int door_zone;
    int top_track_zone;
    int rear_order;
    int front_order;
    int corridor_order;
    int f0128_dispatch_line;
    int f012x_start_line;
    int f012x_f0111_line;
    int lineage_start_line;
    int lineage_rear_order;
    int lineage_top_track_rect;
    int lineage_door_record;
    int lineage_door_state;
    int lineage_door_graphics;
    int lineage_door_rects;
    int lineage_front_order;
    const char *name;
    const char *redmcsb_dispatch;
    const char *lineage_dispatch;
} CSB_V1_ViewportD1LD1RF0111Route;

typedef struct {
    const char *scope;
    const char *redmcsb_dunview_f0111;
    const char *redmcsb_dunview_f0128;
    const char *redmcsb_wall_callers;
    const char *redmcsb_dungeon_zone_math;
    const char *redmcsb_defs;
    const char *csb_lineage_viewport;
} CSB_V1_ViewportD1LD1RF0111Evidence;

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count(void);
const CSB_V1_ViewportD1LD1RF0111Route *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_at(size_t index);
const CSB_V1_ViewportD1LD1RF0111Route *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(int view_square);

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_count(void);
const CSB_V1_ViewportD1LD1RF0111Step *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(size_t route_index,
                                                 size_t step_index);

int csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(int door_state);
int csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state);
int csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state,
    int right_half);
int csb_v1_viewport_d1l_d1r_f0111_door_pc34_apply_c10_blit(
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);
uint32_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_render_hash(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state);

const CSB_V1_ViewportD1LD1RF0111Evidence *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_evidence(void);
const char *csb_v1_viewport_d1l_d1r_f0111_door_pc34_source_lock_header(void);

#ifdef __cplusplus
}
#endif

#endif
