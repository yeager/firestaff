#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D1L/D1R F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors used:
 * - DUNVIEW.C F0108:3940-4011: floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, D1R flip,
 *   and PC34 C1500 + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247: native/flipped
 *   C10 blit paths used by D1 floor, pit, stair, ceiling, and wall pieces.
 * - DUNVIEW.C F0107:3502-3938: wall-ornament keepout for this non-wall
 *   floor+ceiling path.
 * - DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671:
 *   thing-pass cell ordering, row guards, and PC34 view-square gates.
 * - DUNVIEW.C F0122:7391-7557 and F0123:7559-7725: D1L/D1R dispatch,
 *   F0108 at 7525/7693, ceiling copy at 7533/7701, F0115 at 7536/7704.
 * - DUNVIEW.C F0128:8318-8542: near-row dispatch order D1L then D1R
 *   then D1C at 8524-8533.
 * - DUNGEON.C F0163:1769-1838 and F0164:1840-1905: thing-list mutation
 *   anchors; this contract refuses mutation attempts.
 * - DUNGEON.C F0172:2466-2523: square-aspect source for corridor, pit,
 *   teleporter, door-side, and stair-front contexts.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2596-2601 M607/M608 view squares;
 *   2664/2666 cell orders; 2696-2710 wall ordinals; 4045-4046 C705/C706;
 *   4053-4054 C713/C714; 4214-4216 ceiling pit zones; 4223 C1500.
 * CSB-lineage anchors: Viewport.cpp:1167-1189 open F1L1/F1R1 tables,
 * 1892-1925 door-facing side-row contrast, 6507-6548 ApplyDecoration mask,
 * and 7048-7087 D1L/D1R room-slot dispatch with CustomBackgrounds 10/11.
 */

#define CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define CSB_V1_D1L_D1R_F0108_FOOTPRINT_INDEX_PC34 15
#define CSB_V1_D1L_D1R_F0108_C705_ZONE_WALL_D3L_PC34 705
#define CSB_V1_D1L_D1R_F0108_C706_ZONE_WALL_D3R_PC34 706
#define CSB_V1_D1L_D1R_F0108_D1L_FLOOR_ZONE_PC34 1508
#define CSB_V1_D1L_D1R_F0108_D1R_FLOOR_ZONE_PC34 1510

typedef enum {
    CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34 = 1,
    CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 = 2
} CSB_V1_D1LD1RF0108SidePc34;

typedef enum {
    CSB_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    CSB_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    CSB_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    CSB_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34 = 16,
    CSB_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34 = 19
} CSB_V1_D1LD1RF0108ContextPc34;

typedef struct {
    CSB_V1_D1LD1RF0108SidePc34 side;
    const char *name;
    const char *draw_function;
    int relative_depth;
    int relative_lateral;
    int f0128_dispatch_line;
    int lineage_room_slot;
    int lineage_custom_background_line;
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
    int wall_zone;
    int wall_ornament_view;
    int custom_background_mask_after_floor_ceiling;
    int custom_background_uses_mask;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
    const char *lineage_anchor;
} CSB_V1_D1LD1RF0108SpecPc34;

typedef struct {
    CSB_V1_D1LD1RF0108SidePc34 side;
    CSB_V1_D1LD1RF0108ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    int floor_ornament_native_bitmap_index;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t custom_background_pixel;
    uint8_t custom_background_mask;
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
} CSB_V1_D1LD1RF0108StatePc34;

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
} CSB_V1_D1LD1RF0108OrdinalPc34;

typedef struct {
    const CSB_V1_D1LD1RF0108SpecPc34 *spec;
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
    int custom_background_masks;
    int custom_background_after_floor_ceiling;
    int thing_pass_calls;
    int f0115_row_guard_ok;
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
    uint8_t after_custom_background;
    uint8_t after_thing_pass;
    uint32_t deterministic_hash;
} CSB_V1_D1LD1RF0108ResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int d1l_floor_calls;
    int d1r_floor_calls;
    int footprint_recursions;
    int ceiling_copies;
    int custom_background_masks;
    int thing_pass_calls;
    int palette_keepouts;
    int mutation_rejections;
    uint32_t deterministic_hash;
} CSB_V1_D1LD1RF0108SelfTestResultPc34;

size_t csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(void);

const CSB_V1_D1LD1RF0108SpecPc34 *
csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const CSB_V1_D1LD1RF0108SpecPc34 *
csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(int side);

bool csb_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
    CSB_V1_D1LD1RF0108SidePc34 side,
    CSB_V1_D1LD1RF0108ContextPc34 context,
    CSB_V1_D1LD1RF0108StatePc34 *out);

bool csb_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    CSB_V1_D1LD1RF0108OrdinalPc34 *out);

uint8_t csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

uint8_t csb_v1_viewport_d1l_d1r_f0108_apply_custom_background_mask_pc34(
    uint8_t destination_pixel,
    uint8_t background_pixel,
    uint8_t mask);

int csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor);

bool csb_v1_viewport_d1l_d1r_f0108_compose_pc34(
    const CSB_V1_D1LD1RF0108StatePc34 *state,
    CSB_V1_D1LD1RF0108ResultPc34 *out);

int run_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_self_test(void);

const CSB_V1_D1LD1RF0108SelfTestResultPc34 *
csb_v1_viewport_d1l_d1r_f0108_last_self_test_result_pc34(void);

const char *csb_v1_viewport_d1l_d1r_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
