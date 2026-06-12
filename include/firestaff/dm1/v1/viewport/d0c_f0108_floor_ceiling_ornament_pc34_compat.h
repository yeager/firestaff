#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C F0108 floor+ceiling+ornament source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: floor-ornament ordinal handling,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, and PC34
 *   C1500 + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0127:8184-8311: D0C body dispatch; stairs-front breaks
 *   before the shared tail, while corridor/pit/teleporter/door-side reach
 *   F0112 then F0115, with teleporter F0113 after F0115.
 * - DUNVIEW.C F0128:8491-8542: D3L/D3R/D3C dispatch neighborhood,
 *   terminal-depth side-pair correction through D0L/D0R, then D0C.
 * - DUNVIEW.C F0115:4794-4798 and 5245-5267: two-pass door-front drawing
 *   order controlled by L0175_i_DoorFrontViewDrawingPass.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523:
 *   thing-list mutation and square-aspect construction boundaries.
 * - DEFS.H:2533-2559 M550..M558 square-aspect slots, 2680-2702
 *   M575..M579 view-wall ordinals, and 4045-4046 C705/C706 wall zones.
 *
 * This is synthetic metadata/ordering coverage for a 320x200 screen and a
 * 224x136 viewport. It makes no real-asset or original-DOS pixel-parity
 * claim and intentionally does not duplicate the older D0C F0111 or D0C
 * floor-ornament-only keepout gates.
 */

#define DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D0C_F0108_FCO_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0C_F0108_FCO_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0C_F0108_FCO_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_F0108_FCO_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D0C_F0108_FCO_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D0C_F0108_FCO_EXPECTED_HASH_PC34 0x8ef4febeu

typedef enum {
    DM1_V1_D0C_F0108_FCO_CONTEXT_WALL_ORNAMENT_BRANCH_PC34 = 0,
    DM1_V1_D0C_F0108_FCO_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D0C_F0108_FCO_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D0C_F0108_FCO_CONTEXT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D0C_F0108_FCO_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34;

typedef struct {
    const char *name;
    const char *draw_function;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int media720_view_square_d0c;
    int legacy_view_square_d0c;
    int first_thing_slot_pc34;
    int floor_ornament_slot_pc34;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int synthetic_center_coordinate_set;
    int synthetic_center_view_floor;
    int synthetic_center_floor_zone;
    int ceiling_pit_graphic_d0c_pc34;
    int ceiling_pit_zone_d0c_pc34;
    int floor_pit_zone_d0c_pc34;
    int field_zone_d0c_pc34;
    int c705_wall_zone;
    int c706_wall_zone;
    int view_wall_d3l_right;
    int view_wall_d3r_left;
    int view_wall_d3l_front;
    int view_wall_d3c_front;
    int view_wall_d3r_front;
    int f0108_start_line;
    int f0108_end_line;
    int f0127_start_line;
    int f0127_end_line;
    int f0128_d3l_line;
    int f0128_d3r_line;
    int f0128_d3c_line;
    int f0128_d0l_line;
    int f0128_d0r_line;
    int f0128_d0c_line;
    int f0112_ceiling_line;
    int f0115_thing_line;
    int f0113_field_line;
    int door_front_pass_line;
    int door_front_creature_defer_line;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_d0c_body_anchor;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *non_overlap_marker;
} DM1_V1_D0CF0108FloorCeilingOrnamentSpecPc34;

typedef struct {
    DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 context;
    unsigned int floor_ornament_ordinal;
    uint8_t base_floor_pixel;
    uint8_t base_ceiling_pixel;
    uint8_t floor_ornament_pixel;
    uint8_t footprint_pixel;
    uint8_t thing_pixel;
    uint32_t seed;
    bool source_locked_contract_only;
    bool no_original_dos_parity_claim;
    bool no_game_data_load;
    bool request_real_asset_bitmap_compare;
    bool request_f0107_wall_ornament_body;
    bool request_f0111_door_transparency;
    bool request_f0115_thing_pass_detail;
    bool request_sibling_view_slice;
    bool mutate_thing_list;
} DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34;

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
} DM1_V1_D0CF0108FloorCeilingOrnamentOrdinalPc34;

typedef struct {
    const DM1_V1_D0CF0108FloorCeilingOrnamentSpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int f0098_floor_base_calls;
    int f0098_ceiling_base_calls;
    int f0108_reference_locked;
    int f0108_floor_ornament_calls_in_d0c_body;
    int f0108_floor_zone;
    int f0108_primary_index;
    int f0108_recursive_index;
    int f0108_c10_transparency_checks;
    int f0107_wall_ornament_branch_kept_out;
    int f0111_door_transparency_kept_out;
    int f0112_ceiling_calls;
    int f0115_thing_pass_calls;
    int f0113_field_calls;
    int f0112_before_f0115;
    int f0113_after_f0115;
    int terminal_side_pair_correction;
    int d0c_after_d0l_d0r;
    int door_front_two_pass_order_checked;
    int mutation_guard_ok;
    int non_overlap_ok;
    int context_supported;
    uint8_t floor_sample;
    uint8_t ceiling_sample;
    uint8_t ornament_sample_after_c10;
    uint8_t thing_observed_sample;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorCeilingOrnamentResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int contexts_checked;
    int d0c_body_checks;
    int f0108_reference_checks;
    int ordering_checks;
    int c10_checks;
    int mutation_rejections;
    int non_overlap_checks;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34;

const DM1_V1_D0CF0108FloorCeilingOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_spec_pc34(void);

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_initial_state_pc34(
    DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 context,
    DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 *out);

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0CF0108FloorCeilingOrnamentOrdinalPc34 *out);

uint8_t dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_zone_pc34(
    int coordinate_set,
    int view_floor);

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_compose_pc34(
    const DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 *state,
    DM1_V1_D0CF0108FloorCeilingOrnamentResultPc34 *out);

int run_dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34 *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_last_self_test_result_pc34(void);

const char *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_source_evidence_pc34(void);

const char *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_non_overlap_marker_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
