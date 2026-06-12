#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1L2/D1R2 F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors used:
 * - DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling lines 2962-3002:
 *   F0128-owned baseline floor/ceiling copy before the D1 side-lane pass.
 * - DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament lines 3940-4011:
 *   floor-ornament ordinal gate, MASK0x8000_FOOTPRINTS recursion, D1R
 *   horizontal flip, C10 transparent blit, and PC34 C1500 zone math.
 * - DUNVIEW.C F0107 lines 3502-3938: wall-ornament palette branch and
 *   C705/C706 wall-zone band are kept out of this open-floor composition.
 * - DUNVIEW.C F0115 lines 4547-4581 and 5668-5671: separate thing pass,
 *   cell-order loop, and row guard after ceiling copy.
 * - DUNVIEW.C F0122 lines 7391-7557 and F0123 lines 7559-7725: D1L/D1R
 *   dispatch, F0108 floor call, F0112 ceiling copy, and F0115 tail.
 * - DUNVIEW.C F0128 lines 8503-8542: row sweep reaches D2L/D2R, then
 *   D1L/D1R immediately before D1C/D0L/D0R/D0C.
 * - DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1905, and F0172
 *   lines 2466-2523: thing-list mutation anchors and square-aspect source.
 * - DEFS.H lines 2088, 2596-2614, 2662 and 2668-2677, 2681-2707,
 *   4045-4046, 4139-4153, 4202-4207, 4223, and 4239-4254: C10,
 *   I34E/P31J view-square ordinals, cell orders, M575..M579 wall ordinals,
 *   C705/C706 wall zones, floor-ornament zones, and M624 door zones.
 *
 * PASS test_dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat
 */

#define DM1_V1_D1L2_D1R2_F0108_FLOOR_CEILING_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1L2_D1R2_F0108_FLOOR_CEILING_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D1L2_D1R2_F0108_FLOOR_CEILING_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D1L2_D1R2_F0108_SIDE_D1L2_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0108_SIDE_D1R2_PC34 = 2
} DM1_V1_D1L2D1R2F0108SidePc34;

typedef enum {
    DM1_V1_D1L2_D1R2_F0108_SQUARE_WALL_PC34 = 0,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_CORRIDOR_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_PIT_PC34 = 2,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_TELEPORTER_PC34 = 5,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_DOOR_SIDE_PC34 = 16,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_DOOR_FRONT_PC34 = 17,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D1L2_D1R2_F0108_SQUARE_STAIRS_FRONT_PC34 = 19
} DM1_V1_D1L2D1R2F0108SquarePc34;

typedef struct {
    DM1_V1_D1L2D1R2F0108SidePc34 side;
    const char *name;
    const char *draw_function;
    int f0128_dispatch_index;
    int relative_depth;
    int relative_lateral;
    int view_square;
    int view_floor;
    int view_depth;
    int view_lane;
    int floor_ornament_native_increment;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int floor_view_flipped_by_f0108;
    int ceiling_graphic;
    int ceiling_zone;
    int ceiling_flip_horizontal;
    int thing_pass_order;
    int thing_pass_view_square;
    int g2028_row;
    int g2033_row;
    int g2034_row;
    int wall_ornament_view;
    int m575_view_wall_d3l_right;
    int m576_view_wall_d3r_left;
    int m577_view_wall_d3l_front;
    int m578_view_wall_d3c_front;
    int m579_view_wall_d3r_front;
    int f0674_f0675_dispatch_entry;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D1L2D1R2F0108SpecPc34;

typedef struct {
    DM1_V1_D1L2D1R2F0108SidePc34 side;
    int square_element;
    unsigned int floor_ornament_ordinal;
    int floor_ornament_coordinate_set;
    int floor_ornament_native_bitmap_index;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t thing_pass_pixel;
    uint32_t seed;
    uint32_t mutation_guard_before;
    uint32_t mutation_guard_after;
    bool contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool attempts_f0107_wall_ornament;
    bool attempts_f0111_door;
    bool mutate_thing_list;
} DM1_V1_D1L2D1R2F0108StatePc34;

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
    int metadata_blit_count;
} DM1_V1_D1L2D1R2F0108OrdinalPc34;

typedef struct {
    const DM1_V1_D1L2D1R2F0108SpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0108_floor_calls;
    int f0108_primary_blits;
    int f0108_footprint_recursions;
    int f0098_floor_ceiling_base_calls;
    int ceiling_copy_calls;
    int thing_pass_calls;
    int dispatch_entries;
    int f0674_f0675_dispatch_entries;
    int row_guard_accepts;
    int mutation_rejections;
    int f0107_keepout_ok;
    int f0107_palette_keepout_ok;
    int f0111_keepout_ok;
    int m575_to_m579_ordinal_parity;
    int f0115_first_cell;
    int f0115_second_cell;
    int floor_zone;
    int floor_primary_index;
    int floor_recursive_index;
    int call_order_base_before_floor;
    int call_order_floor_before_ceiling;
    int call_order_ceiling_before_thing_pass;
    uint8_t after_floor;
    uint8_t after_ceiling;
    uint8_t after_thing_pass;
    uint32_t deterministic_hash;
} DM1_V1_D1L2D1R2F0108ResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int floor_recursion_calls;
    int ceiling_copies;
    int thing_pass_calls;
    int dispatch_entries;
    int row_guard_rejections;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D1L2D1R2F0108SelfTestResultPc34;

size_t dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_count_pc34(void);

const DM1_V1_D1L2D1R2F0108SpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const DM1_V1_D1L2D1R2F0108SpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_for_side_pc34(int side);

bool dm1_v1_viewport_d1l2_d1r2_f0108_initial_state_pc34(
    DM1_V1_D1L2D1R2F0108SidePc34 side,
    DM1_V1_D1L2D1R2F0108StatePc34 *out);

bool dm1_v1_viewport_d1l2_d1r2_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D1L2D1R2F0108OrdinalPc34 *out);

uint8_t dm1_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d1l2_d1r2_f0108_flip_row_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t rows);

int dm1_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(
    int view_square,
    int view_cell,
    int view_depth);

int dm1_v1_viewport_d1l2_d1r2_f0108_view_wall_ordinal_pc34(int ordinal);

bool dm1_v1_viewport_d1l2_d1r2_f0108_compose_pc34(
    const DM1_V1_D1L2D1R2F0108StatePc34 *state,
    DM1_V1_D1L2D1R2F0108ResultPc34 *out);

int run_dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D1L2D1R2F0108SelfTestResultPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d1l2_d1r2_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
