#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34 0x4000
#define CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_MASK0X8000_PC34 0x8000

typedef enum {
    CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_SIDE_D0L2_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_SIDE_D0R2_PC34 = 2
} CSB_V1_D0L2D0R2F0111PartlyOpenDoorSidePc34;

typedef enum {
    CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_OPEN_PC34 = 0,
    CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_CLOSED_PC34 = 2,
    CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_DESTROYED_PC34 = 3,
    CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34 = -1
} CSB_V1_D0L2D0R2F0111DoorBranchPc34;

typedef enum {
    CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_MASK_AFTER_FLOOR_CEILING_PC34 = 0,
    CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_ROOM_BITMAP_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0115_REAR_OBJECTS_PC34 = 2,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_FIRST_HALF_PC34 = 3,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_SECOND_HALF_PC34 = 4,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0115_FRONT_OBJECTS_PC34 = 5
} CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int view_square;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0125_f0126_function_number;
    int d0r_uses_horizontal_flip;
    int wall_zone;
    int door_zone_base;
    int rear_cell_order;
    int front_cell_order;
    int open_state;
    int partly_open_state_one;
    int partly_open_state_two;
    int partly_open_state_three;
    int closed_state;
    int destroyed_state;
    int open_fraction_denominator;
    int first_half_zone_offset;
    int second_half_zone_offset;
    int second_half_zone_mask;
    int c10_transparent_color;
    int mask0x8000_footprint_recursion_keepout;
    int wall_ornament_keepout;
    int floor_ornament_keepout;
    int custom_backgrounds_before_door;
    int f0163_not_called_by_draw;
    int f0164_not_called_by_draw;
    const char *left_horizontal_frame_bitmap;
    const char *right_horizontal_frame_bitmap;
    const char *f0111_anchor;
    const char *f0104_anchor;
    const char *f0105_anchor;
    const char *f0107_anchor;
    const char *f0108_anchor;
    const char *f0115_anchor;
    const char *f0128_anchor;
    const char *dungeon_anchor;
    const char *defs_anchor;
    const char *lineage_anchor;
    const char *custom_backgrounds_anchor;
} CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34;

typedef struct {
    int ok;
    int copied_pixels;
    int c10_skipped_pixels;
    int row_guard_rejections;
    int mutation_rejections;
    int footprint_recursions;
    int left_edge_writes;
    int right_edge_writes;
    uint32_t deterministic_hash;
} CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34;

typedef struct {
    int route_count;
    int d0r_flip_ok;
    int partly_open_gate_ok;
    int custom_backgrounds_depth_ok;
    int wall_keepout_ok;
    int floor_keepout_ok;
    int first_half_zone;
    int second_half_zone;
    int copied_pixels;
    int c10_skipped_pixels;
    int row_guard_rejections;
    int mutation_rejections;
    uint32_t deterministic_hash;
} CSB_V1_D0L2D0R2F0111PartlyOpenDoorProbePc34;

size_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(void);

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(size_t index);

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(int side);

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_open_fraction_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

const char *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(
    unsigned int order,
    int ordinal);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int source_width,
    int x);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int wall_ornament_ordinal,
    int door_composition_active);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int floor_ornament_ordinal,
    int door_composition_active,
    int *out_footprint_recursions);

size_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_order_pc34(
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34 *out_steps,
    size_t out_capacity);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34 *out_result);

uint32_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34(void);

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorProbePc34 *out_probe);

const char *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
