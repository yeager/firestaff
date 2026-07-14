#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34 = 1
} DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34;

typedef enum {
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_CORRIDOR_PC34 = 1,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_PIT_PC34 = 2,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_TELEPORTER_PC34 = 5,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_DOOR_SIDE_PC34 = 16,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_WALL_PC34 = 0,
    DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_DOOR_FRONT_PC34 = 17
} DM1_V1_D3L2D3R2F0108FloorCeilingSquarePc34;

typedef struct {
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side;
    const char *label;
    int view_square;
    int view_floor;
    int view_depth;
    int view_lane;
    int wall_zone;
    int floor_zone_base;
    int floor_zone_stride;
    unsigned int open_cell_order;
    unsigned int door_pass1_cell_order;
    unsigned int door_pass2_cell_order;
    bool f0108_right_side_flips;
    bool f0099_row_local_flip_preserved;
    bool f0128_dispatch_after_d3c;
    bool f0115_contract_external;
    const char *redmcsb_f067x_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0099_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34;

typedef struct {
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side;
    int square_element;
    bool post_d3c_reached;
    bool enable_f0108_floor_ornament;
    bool enable_f0098_floor_fallback;
    bool enable_f0098_ceiling_fallback;
    bool f0115_thing_pass_already_covered;
    bool attempts_f0107_wall_ornament;
    bool attempts_f0111_door;
    unsigned int floor_ornament_ordinal;
    int floor_ornament_coordinate_set;
    int floor_ornament_native_bitmap_index;
    uint8_t destination_pixel;
    uint8_t f0098_floor_pixel;
    uint8_t f0098_ceiling_pixel;
    uint8_t f0108_ornament_pixel;
    uint8_t f0115_synthetic_pixel;
    uint32_t mutation_guard_before;
    uint32_t mutation_guard_after;
} DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34;

typedef struct {
    unsigned int input_ordinal;
    bool has_input_ordinal;
    bool footprint_flag_set;
    unsigned int cleared_ordinal;
    bool primary_draws;
    unsigned int primary_ordinal;
    int primary_index;
    bool recursive_footprints_draw;
    unsigned int recursive_footprints_ordinal;
    int recursive_footprints_index;
} DM1_V1_D3L2D3R2F0108FloorCeilingOrdinalPc34;

typedef struct {
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *spec;
    int f0098FloorCount;
    int f0098CeilingCount;
    int f0099FlipCount;
    int f0108OrnamentCount;
    int f0115ThingPassNoOpCount;
    int c10TransparentBlitCount;
    int f0676FrameCount;
    int f0677FrameCount;
    int f0128PostD3cCount;
    int mutationGuardsOk;
    int f0107NonOverlapOk;
    int f0111NonOverlapOk;
    int nonOverlapWithF0107F0111;
    int rejectsNonContractState;
    int floor_ornament_zone;
    int floor_ornament_primary_index;
    int footprint_recursion_index;
    int view_square;
    int view_floor;
    int open_cell_order;
    bool row_local_parity_ok;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    uint8_t after_f0098_floor;
    uint8_t after_f0108_ornament;
    uint8_t after_f0098_ceiling;
    uint8_t after_f0115_noop;
} DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34;

size_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_count_pc34(void);

const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side);

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side,
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 *out);

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D3L2D3R2F0108FloorCeilingOrdinalPc34 *out);

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t row_count);

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(
    const DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 *state,
    DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34 *out);

const char *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
