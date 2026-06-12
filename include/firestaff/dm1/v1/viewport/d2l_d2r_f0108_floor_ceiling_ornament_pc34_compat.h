#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2L/D2R F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, and PC34
 *   C1500 + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0119:6987-7031: D2L door-front/corridor/pit routes draw
 *   F0108 on M591, then ceiling C864, then F0115 on M604.
 * - DUNVIEW.C F0120:7180-7224: D2R mirror route draws F0108 on M593,
 *   flipped ceiling C866, and F0115 on M605.
 * - DUNVIEW.C F0128:8503-8517: D2L2/D2R2 lateral-2 pass is completed
 *   before the D2L/D2R F0108-capable near-side squares.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523:
 *   thing-list mutation and square-aspect construction anchors.
 *
 * This is a metadata/ordering contract only; it makes no original DOS parity
 * claim and performs no real game-data bitmap comparison.
 */

#define DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L_D2R_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D2L_D2R_F0108_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D2L_D2R_F0108_SIDE_D2L_PC34 = 1,
    DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 = 2
} DM1_V1_D2LD2RF0108SidePc34;

typedef enum {
    DM1_V1_D2L_D2R_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D2L_D2R_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D2L_D2R_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D2L_D2R_F0108_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D2LD2RF0108ContextPc34;

typedef struct {
    DM1_V1_D2LD2RF0108SidePc34 side;
    const char *name;
    const char *draw_function;
    int f0128_dispatch_order;
    int relative_forward;
    int relative_lateral;
    int owner_view_square;
    int view_floor;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int floor_zone;
    int right_side_flip;
    int ceiling_graphic;
    int ceiling_zone;
    int ceiling_flip_horizontal;
    int corridor_order;
    int door_pass1_order;
    int door_pass2_order;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_dungeon_anchor;
} DM1_V1_D2LD2RF0108SpecPc34;

typedef struct {
    DM1_V1_D2LD2RF0108SidePc34 side;
    DM1_V1_D2LD2RF0108ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t rear_thing_pixel;
    uint8_t front_thing_pixel;
    bool source_locked_contract_only;
    bool no_original_dos_parity_claim;
    bool no_game_data_load;
    bool mutate_thing_list;
    bool allow_wall_ornament_overlap;
    bool allow_door_overlap;
} DM1_V1_D2LD2RF0108StatePc34;

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
} DM1_V1_D2LD2RF0108OrdinalPc34;

typedef struct {
    const DM1_V1_D2LD2RF0108SpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int floor_ornament_calls;
    int floor_primary_blits;
    int footprint_recursions;
    int c10_transparent_blits;
    int ceiling_calls;
    int rear_thing_pass_calls;
    int front_thing_pass_calls;
    int open_pit_still_draws_floor_ornament;
    int lateral2_prepass_complete;
    int wall_door_keepout_ok;
    int mutation_guard_ok;
    int floor_zone;
    int floor_primary_index;
    int floor_recursive_index;
    uint8_t after_floor;
    uint8_t after_ceiling;
    uint8_t after_front_thing;
    uint32_t deterministic_hash;
} DM1_V1_D2LD2RF0108ResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int d2l_floor_calls;
    int d2r_floor_calls;
    int footprint_recursions;
    int ceiling_calls;
    int thing_pass_calls;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D2LD2RF0108SelfTestResultPc34;

size_t dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_count_pc34(void);

const DM1_V1_D2LD2RF0108SpecPc34 *
dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const DM1_V1_D2LD2RF0108SpecPc34 *
dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_for_side_pc34(int side);

bool dm1_v1_viewport_d2l_d2r_f0108_initial_state_pc34(
    DM1_V1_D2LD2RF0108SidePc34 side,
    DM1_V1_D2LD2RF0108ContextPc34 context,
    DM1_V1_D2LD2RF0108StatePc34 *out);

bool dm1_v1_viewport_d2l_d2r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2LD2RF0108OrdinalPc34 *out);

uint8_t dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d2l_d2r_f0108_compose_pc34(
    const DM1_V1_D2LD2RF0108StatePc34 *state,
    DM1_V1_D2LD2RF0108ResultPc34 *out);

int run_dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D2LD2RF0108SelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_f0108_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d2l_d2r_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
