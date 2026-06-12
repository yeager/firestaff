#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1L2/D1R2 F0108 wall-band composition source lock.
 *
 * This locks the wall-square route where F0122/F0123 draw the D1 side wall
 * and call F0107 for the side wall ornament, while explicitly keeping the
 * F0108 floor-ornament band out of the wall composition.
 */

#define DM1_V1_D1L2_D1R2_F0108_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D1L2_D1R2_F0108_WALL_SURFACE_BYTES_PC34 8

typedef enum {
    DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1L2_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 = 2
} DM1_V1_D1L2D1R2F0108WallSidePc34;

typedef enum {
    DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_NATIVE_PC34 = 0,
    DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_F0128_FLIPPED_PC34 = 1
} DM1_V1_D1L2D1R2F0108WallRoutePc34;

typedef struct {
    DM1_V1_D1L2D1R2F0108WallSidePc34 side;
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route;
    const char *name;
    int draw_order_index;
    int relative_depth;
    int relative_lateral;
    int view_square;
    int view_wall;
    int view_floor_keepout;
    int wall_zone;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride_pc34;
    int floor_keepout_zone_base;
    int floor_keepout_zone_stride_pc34;
    int wall_element;
    int wall_set_native_index;
    int wall_set_flipped_index;
    int uses_f0104_native_blit;
    int uses_f0105_flipped_blit;
    int d1r_horizontal_flip_contract;
    int f0128_global_flip_contract;
    int cell_order_wall_return;
    int cell_order_open;
    int cell_order_door_pass1;
    int cell_order_door_pass2;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_wall_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_keepout_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D1L2D1R2F0108WallSpecPc34;

typedef struct {
    DM1_V1_D1L2D1R2F0108WallSidePc34 side;
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route;
    unsigned int wall_ornament_ordinal;
    unsigned int floor_ornament_ordinal;
    int wall_ornament_coordinate_set;
    int floor_ornament_coordinate_set;
    int view_cell;
    int view_depth;
    uint8_t destination_pixel;
    uint8_t wall_pixel;
    uint8_t footprint_pixel;
    uint32_t seed;
    bool contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool attempts_f0108_floor_band;
    bool mutate_thing_list;
} DM1_V1_D1L2D1R2F0108WallStatePc34;

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
} DM1_V1_D1L2D1R2F0108WallOrdinalPc34;

typedef struct {
    const DM1_V1_D1L2D1R2F0108WallSpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int wall_ordinal_zero_skip;
    int f0107_wall_calls;
    int f0107_primary_blits;
    int footprint_recursions;
    int f0108_floor_calls;
    int f0108_floor_keepout_ok;
    int row_guard_accepts;
    int row_guard_rejections;
    int mutation_rejections;
    int caller_surface_mutations;
    int d1r_horizontal_flip_observed;
    int f0128_global_flip_observed;
    int cell_order_transition_ok;
    int wall_zone;
    int floor_keepout_zone;
    int primary_index;
    int recursive_index;
    uint8_t after_wall;
    uint8_t after_footprint;
    uint32_t deterministic_hash;
} DM1_V1_D1L2D1R2F0108WallResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int wall_draws;
    int footprint_recursions;
    int row_guard_rejections;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D1L2D1R2F0108WallSelfTestResultPc34;

size_t dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_count_pc34(void);

const DM1_V1_D1L2D1R2F0108WallSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_at_pc34(size_t index);

const DM1_V1_D1L2D1R2F0108WallSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_for_pc34(
    DM1_V1_D1L2D1R2F0108WallSidePc34 side,
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route);

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_initial_state_pc34(
    DM1_V1_D1L2D1R2F0108WallSidePc34 side,
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route,
    DM1_V1_D1L2D1R2F0108WallStatePc34 *out);

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int wall_ornament_ordinal,
    DM1_V1_D1L2D1R2F0108WallOrdinalPc34 *out);

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(
    int coordinate_set,
    int view_wall);

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(
    int coordinate_set,
    int view_floor);

uint8_t dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_flip_row_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t rows);

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
    int view_square,
    int view_cell,
    int view_depth);

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
    const DM1_V1_D1L2D1R2F0108WallStatePc34 *state,
    uint8_t *caller_surface,
    size_t caller_surface_size,
    DM1_V1_D1L2D1R2F0108WallResultPc34 *out);

int run_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_self_test(void);

const DM1_V1_D1L2D1R2F0108WallSelfTestResultPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
