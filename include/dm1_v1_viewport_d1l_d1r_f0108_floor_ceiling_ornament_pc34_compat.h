#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1L/D1R F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors used:
 * - DUNVIEW.C F0108:3940-4011: floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, and PC34
 *   C1500 + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247: native and flipped
 *   C10 blit paths used by D1 side floor, stair, pit, and ceiling pieces.
 * - DUNVIEW.C F0107:3502-3938: wall-ornament keepout for non-wall cases.
 * - DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214: thing-pass
 *   cell ordering, object C10 blit, and PC34 view-square/cell gates.
 * - DUNVIEW.C F0127 and F0128:8318-8486,8536-8541: dispatcher sweep
 *   anchor; D1L/D1R are the depth-1 single-step side walls.
 * - DUNGEON.C F0163:1769-1838 and F0164:1840-1905: thing-list mutation
 *   anchors; this contract refuses mutation attempts.
 * - DUNGEON.C F0172:2466-2523: square-aspect source for corridor, pit,
 *   teleporter, door-side, and stair-front contexts.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2596-2611 view squares; 2662 and
 *   2668-2677 cell orders; 4045-4046/C705/C706 wall-zone band;
 *   4139-4153 cell-order zone band; 4223 C1500_ZONE_FLOOR_ORNAMENT.
 *
 * Contract flags: source_locked_contract_only=1, no_real_asset_bitmap_parity=1,
 * no_game_data_load=1.
 */

#define DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D1L_D1R_F0108_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34 = 1,
    DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 = 2
} DM1_V1_D1LD1RF0108SidePc34;

typedef enum {
    DM1_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D1LD1RF0108ContextPc34;

typedef struct {
    DM1_V1_D1LD1RF0108SidePc34 side;
    const char *name;
    const char *draw_function;
    int relative_depth;
    int relative_lateral;
    int f0128_dispatch_line;
    int view_square;
    int view_floor;
    int floor_zone;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int floor_coordinate_set;
    int floor_flip_horizontal;
    int ceiling_graphic;
    int ceiling_zone;
    int ceiling_flip_horizontal;
    int thing_pass_order;
    int thing_pass_view_square;
    int thing_pass_first_cell;
    int thing_pass_second_cell;
    int wall_keepout_zone_first;
    int wall_keepout_zone_last;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D1LD1RF0108SpecPc34;

typedef struct {
    DM1_V1_D1LD1RF0108SidePc34 side;
    DM1_V1_D1LD1RF0108ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    int floor_ornament_native_bitmap_index;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t thing_pass_pixel;
    uint32_t seed;
    uint32_t mutation_guard_before;
    uint32_t mutation_guard_after;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool attempts_f0107_wall_ornament;
    bool attempts_f0111_door;
    bool mutate_thing_list;
} DM1_V1_D1LD1RF0108StatePc34;

typedef struct {
    unsigned int input_ordinal;
    bool has_input_ordinal;
    bool footprint_flag_set;
    unsigned int cleared_ordinal;
    bool primary_draws;
    int primary_index;
    bool recursive_footprints_draw;
    int recursive_footprints_index;
    int metadata_blit_count;
} DM1_V1_D1LD1RF0108OrdinalPc34;

typedef struct {
    const DM1_V1_D1LD1RF0108SpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0108_floor_calls;
    int f0108_primary_blits;
    int f0108_footprint_recursions;
    int c10_transparent_blits;
    int ceiling_copy_calls;
    int thing_pass_calls;
    int cell_order_band_ok;
    int palette_keepout_ok;
    int mutation_rejections;
    int f0107_keepout_ok;
    int f0111_keepout_ok;
    int floor_zone;
    int floor_primary_index;
    int floor_recursive_index;
    uint8_t after_floor;
    uint8_t after_ceiling;
    uint8_t after_thing_pass;
    uint32_t deterministic_hash;
} DM1_V1_D1LD1RF0108ResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int d1l_floor_calls;
    int d1r_floor_calls;
    int footprint_recursions;
    int thing_pass_calls;
    int palette_keepouts;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D1LD1RF0108SelfTestResultPc34;

size_t dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(void);

const DM1_V1_D1LD1RF0108SpecPc34 *
dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const DM1_V1_D1LD1RF0108SpecPc34 *
dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(int side);

bool dm1_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
    DM1_V1_D1LD1RF0108SidePc34 side,
    DM1_V1_D1LD1RF0108ContextPc34 context,
    DM1_V1_D1LD1RF0108StatePc34 *out);

bool dm1_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D1LD1RF0108OrdinalPc34 *out);

uint8_t dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(
    const DM1_V1_D1LD1RF0108StatePc34 *state,
    DM1_V1_D1LD1RF0108ResultPc34 *out);

int run_dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D1LD1RF0108SelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_f0108_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d1l_d1r_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
