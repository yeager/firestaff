#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3L/D3R F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: M558 floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, right-side
 *   D3R flip branch, and PC34 C1500 + CoordinateSet * 11 + ViewFloor math.
 * - DUNVIEW.C F0116:6361-6498: D3L body carries F0108 at 6443/6478,
 *   C705 wall zone, M575/M577 wall-ornament calls, and C0x3421 ordering.
 * - DUNVIEW.C F0117:6500-6640: D3R partner carries F0108 at 6579/6620,
 *   C706 wall zone, M576/M579 wall-ornament calls, and C0x4312 ordering.
 * - DUNVIEW.C F0128:8491-8517: dispatch is D3L, D3R, D3C, then the
 *   later D2L/D2R pair.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523:
 *   thing-list mutation boundaries and square-aspect construction.
 * - DEFS.H C0..C5 wall-ornament ordinals, M575..M579 view-wall positions,
 *   C705/C706 wall zones, C1004 wall-ornament base, and C1500 floor base.
 *
 * The probe uses a synthetic 320x200 framebuffer with a 224x136 viewport.
 * It proves source-lock metadata and ordering only; it makes no original
 * DOS pixel-parity or real-asset bitmap claim.
 *
 * Non-overlap marker: pass770 owns only D3L/D3R F0108 floor+ceiling+
 * ornament. It does not duplicate D0C, D0L/D0R, D0L2/D0R2, D1C, D1L/D1R,
 * D1L2/D1R2, D2L/D2R, D2L2/D2R2, or D3C F0108 gates, and does not
 * duplicate the D3L/D3R F0107 wall-ornament gate.
 */

#define DM1_V1_D3L_D3R_F0108_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D3L_D3R_F0108_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D3L_D3R_F0108_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L_D3R_F0108_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L_D3R_F0108_SIDE_COUNT_PC34 2
#define DM1_V1_D3L_D3R_F0108_CONTEXT_COUNT_PC34 8
#define DM1_V1_D3L_D3R_F0108_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L_D3R_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3L_D3R_F0108_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34 = 1,
    DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 = 2
} DM1_V1_D3LD3RF0108SidePc34;

typedef enum {
    DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34 = 0,
    DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D3L_D3R_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D3LD3RF0108ContextPc34;

typedef struct {
    DM1_V1_D3LD3RF0108SidePc34 side;
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
    int wall_zone;
    int side_view_wall;
    int front_view_wall;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride_pc34;
    int side_wall_ornament_zone;
    int front_wall_ornament_zone;
    int corridor_order;
    int door_side_order;
    int door_pass1_order;
    int door_pass2_order;
    int f0108_door_front_line;
    int f0108_open_path_line;
    int f0115_line;
    int f0113_field_line;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_body_anchor;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *non_overlap_marker;
} DM1_V1_D3LD3RF0108SpecPc34;

typedef struct {
    DM1_V1_D3LD3RF0108SidePc34 side;
    DM1_V1_D3LD3RF0108ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    uint8_t wall_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t ornament_pixel;
    uint8_t thing_pixel;
    bool source_locked_contract_only;
    bool no_original_dos_parity_claim;
    bool no_game_data_load;
    bool mutate_thing_list;
    bool allow_sibling_f0108_overlap;
    bool allow_f0107_wall_ornament_duplicate;
    bool allow_outside_viewport;
} DM1_V1_D3LD3RF0108StatePc34;

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
} DM1_V1_D3LD3RF0108OrdinalPc34;

typedef struct {
    const DM1_V1_D3LD3RF0108SpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int floor_ornament_calls;
    int floor_primary_blits;
    int footprint_recursions;
    int c10_transparent_blits;
    int floor_base_calls;
    int ceiling_base_calls;
    int wall_body_calls;
    int wall_zone;
    int side_wall_ornament_zone;
    int front_wall_ornament_zone;
    int rear_thing_pass_calls;
    int front_thing_pass_calls;
    int open_pit_still_draws_floor_ornament;
    int f0128_d3l_then_d3r_then_d3c;
    int terminal_depth_d2_pair_drawn_later;
    int mutation_guard_ok;
    int non_overlap_ok;
    int floor_zone;
    int floor_primary_index;
    int floor_recursive_index;
    uint8_t ceiling_sample;
    uint8_t floor_sample;
    uint8_t ornament_sample_after_c10;
    uint8_t d2_later_sample;
    uint32_t deterministic_hash;
} DM1_V1_D3LD3RF0108ResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int d3l_floor_calls;
    int d3r_floor_calls;
    int wall_body_calls;
    int footprint_recursions;
    int ceiling_calls;
    int thing_pass_calls;
    int mutation_rejections;
    int non_overlap_assertions;
    uint32_t deterministic_hash;
} DM1_V1_D3LD3RF0108SelfTestResultPc34;

size_t dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34(void);

const DM1_V1_D3LD3RF0108SpecPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_at_pc34(size_t index);

const DM1_V1_D3LD3RF0108SpecPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34(int side);

bool dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(
    DM1_V1_D3LD3RF0108SidePc34 side,
    DM1_V1_D3LD3RF0108ContextPc34 context,
    DM1_V1_D3LD3RF0108StatePc34 *out);

bool dm1_v1_viewport_d3l_d3r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D3LD3RF0108OrdinalPc34 *out);

uint8_t dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d3l_d3r_f0108_floor_zone_pc34(
    int coordinate_set,
    int view_floor);

int dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall);

bool dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(
    const DM1_V1_D3LD3RF0108StatePc34 *state,
    DM1_V1_D3LD3RF0108ResultPc34 *out);

int run_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D3LD3RF0108SelfTestResultPc34 *
dm1_v1_viewport_d3l_d3r_f0108_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d3l_d3r_f0108_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3l_d3r_f0108_non_overlap_marker_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
