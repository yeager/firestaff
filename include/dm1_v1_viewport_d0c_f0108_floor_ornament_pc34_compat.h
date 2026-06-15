#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C F0108 floor-ornament composition source lock.
 *
 * D0C reaches F0127, whose current-square foreground/ceiling/thing/field
 * sequence never dispatches F0108. This gate still source-locks the F0108
 * ordinal, C10, footprint, and zone contracts so D0C stays an explicit
 * no-floor-ornament lane instead of silently borrowing a D1/D2/D3 floor view.
 */

#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_NO_VIEW_FLOOR_PC34 (-1)
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34 8
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 0x87980ce5u

typedef enum {
    DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D0CF0108FloorOrnamentContextPc34;

typedef struct {
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context;
    const char *name;
    int square_element;
    int draw_order_index;
    int relative_depth;
    int relative_lateral;
    int view_square;
    int view_floor;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int floor_ornament_aspect_slot;
    int calls_f0108_floor_ornament;
    int reads_floor_ornament_slot;
    int foreground_before_thing_pass;
    int ceiling_before_thing_pass;
    int field_after_thing_pass;
    int thing_pass_order;
    int thing_pass_view_square;
    int wall_ornament_palette_keepout;
    int wall_keepout_zone_left;
    int wall_keepout_zone_right;
    int cell_order_band_start;
    int cell_order_band_end;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D0CF0108FloorOrnamentSpecPc34;

typedef struct {
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context;
    unsigned int floor_ornament_ordinal;
    int floor_ornament_coordinate_set;
    uint8_t destination_pixel;
    uint8_t hypothetical_floor_pixel;
    uint8_t foreground_pixel;
    uint8_t ceiling_pixel;
    uint8_t thing_pass_pixel;
    uint8_t field_pixel;
    uint32_t seed;
    bool teleporter_visible;
    bool contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool attempts_f0108_floor_ornament;
    bool attempts_f0107_wall_ornament;
    bool mutate_thing_list;
} DM1_V1_D0CF0108FloorOrnamentStatePc34;

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
} DM1_V1_D0CF0108FloorOrnamentOrdinalPc34;

typedef struct {
    const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0108_floor_calls;
    int f0108_primary_blits;
    int f0108_footprint_recursions;
    int f0108_d0c_no_view_floor_keepout_ok;
    int f0107_palette_keepout_ok;
    int thing_pass_calls;
    int foreground_calls;
    int ceiling_calls;
    int field_calls;
    int mutation_rejections;
    int floor_zone;
    int floor_primary_index;
    int floor_recursive_index;
    int cell_order_transition_ok;
    int caller_surface_mutations;
    uint8_t after_foreground;
    uint8_t after_ceiling;
    uint8_t after_thing_pass;
    uint8_t after_field;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorOrnamentResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int d0c_keepout_compositions;
    int thing_pass_calls;
    int palette_keepouts;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34;

size_t dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat(void);

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_at_pc34_compat(size_t index);

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_for_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context,
    DM1_V1_D0CF0108FloorOrnamentStatePc34 *out);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 *out);

int dm1_v1_viewport_d0c_f0108_floor_ornament_zone_pc34_compat(
    int coordinate_set,
    int view_floor);

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_flip_row_pc34_compat(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t rows);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
    const DM1_V1_D0CF0108FloorOrnamentStatePc34 *state,
    uint8_t *caller_surface,
    size_t caller_surface_size,
    DM1_V1_D0CF0108FloorOrnamentResultPc34 *out);

int run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 *out);

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
