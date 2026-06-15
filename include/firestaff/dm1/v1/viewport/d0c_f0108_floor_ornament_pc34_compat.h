#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C F0108 floor-ornament source-lock probe.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011 MASK0x8000_FOOTPRINTS, C10 transparent
 *   floor-ornament blit, and PC34 C1500 + CoordinateSet * 11 + ViewFloor.
 * - DUNVIEW.C F0107:3502-3938 wall-ornament palette/zone keepout.
 * - DUNVIEW.C F0098:2962-3002 floor+ceiling base before square passes.
 * - DUNVIEW.C F0115:4547-4581,5180-5188,5211-5214,5668-5671 thing-pass
 *   cell processing and C10 transparent object/projectile blits.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2596-2611 I34E/P31J view-square
 *   ordinals; 2668-2677 and 2698-2702 cell/view ordinals; 4045-4046
 *   C705/C706 wall zones.
 */

#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34 8
#define DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 0x537d38aeu

typedef struct {
    const char *name;
    int view_square_d0c;
    int central_floor_view;
    int floor_zone_base;
    int floor_zone_stride_pc34;
    int floor_coordinate_set;
    int floor_zone;
    int f0098_base_order;
    int f0108_floor_ornament_order;
    int f0107_keepout_order;
    int f0115_thing_pass_order;
    int thing_pass_cell_order;
    int thing_pass_view_square;
    int wall_keepout_zone_left;
    int wall_keepout_zone_right;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D0CF0108FloorOrnamentSpecPc34;

typedef struct {
    unsigned int floor_ornament_ordinal;
    uint8_t base_pixel;
    uint8_t floor_ornament_pixel;
    uint8_t footprint_pixel;
    uint8_t f0107_keepout_pixel;
    uint8_t thing_pass_pixel;
    uint32_t seed;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool attempts_floor_ceiling_composite;
    bool attempts_d0l_d0r_route;
    bool attempts_wall_ornament_write;
    bool mutate_thing_list;
} DM1_V1_D0CF0108FloorOrnamentStatePc34;

typedef struct {
    unsigned int input_ordinal;
    bool has_input_ordinal;
    bool footprint_flag_set;
    unsigned int cleared_ordinal;
    bool primary_draws;
    int primary_index;
    bool recursive_footprints_draw;
    int recursive_footprints_index;
} DM1_V1_D0CF0108FloorOrnamentOrdinalPc34;

typedef struct {
    const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0098_base_writes;
    int f0108_floor_ornament_writes;
    int f0108_primary_blits;
    int f0108_footprint_recursions;
    int f0107_keepout_writes;
    int f0107_keepout_preserved_floor;
    int thing_pass_calls;
    int thing_pass_observed_floor;
    int floor_write_before_thing_pass;
    int central_cell_only;
    int mutation_rejections;
    uint8_t after_f0098_base;
    uint8_t after_f0108_floor;
    uint8_t after_f0108_footprints;
    uint8_t after_f0107_keepout;
    uint8_t thing_pass_observed_pixel;
    uint8_t after_thing_pass;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorOrnamentResultPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int floor_writes;
    int thing_pass_calls;
    int keepout_preservations;
    int mutation_rejections;
    uint32_t deterministic_hash;
} DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34;

typedef struct {
    const char *name;
    int source_locked_contract_only;
    int no_original_dos_parity_claim;
    int square_element_pit;
    int pit_open_mask;
    int pit_invisible_mask;
    int pit_visible_aspect_slot_pc34;
    int floor_ornament_aspect_slot_pc34;
    int sensor_floor_ornament_ordinal;
    int is_kappetaal_pit_boundary_variant;
    int is_regular_floor_ornament_variant;
    int f0108_floor_ornament_calls;
    int f0108_floor_ornament_zone;
    int g0206_floor_ornament_coordinate_route;
    int g0207_door_ornament_route;
    int g0208_door_button_route;
    int d0c_view_square;
    int d0c_cell_order;
    int floor_pit_graphic_open_pc34;
    int floor_pit_graphic_invisible_pc34;
    int floor_pit_zone_pc34;
    int ceiling_pit_graphic_pc34;
    int ceiling_pit_zone_pc34;
    int viewport_width;
    int viewport_height;
    int screen_width;
    int screen_height;
    int open_pit_dst_x;
    int open_pit_dst_y;
    int open_pit_dst_w;
    int open_pit_dst_h;
    int invisible_pit_dst_x;
    int invisible_pit_dst_y;
    int invisible_pit_dst_w;
    int invisible_pit_dst_h;
    int geometry_inside_viewport;
    int graphics_dat_asset_route;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0172_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_coordinate_anchor;
    const char *firestaff_graphics_dat_anchor;
} DM1_V1_D0CF0108FloorOrnamentKappetaalVariantPc34;

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_spec_pc34_compat(void);

const DM1_V1_D0CF0108FloorOrnamentKappetaalVariantPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_kappetaal_variant_pc34_compat(void);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentStatePc34 *out);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 *out);

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
    const DM1_V1_D0CF0108FloorOrnamentStatePc34 *state,
    uint8_t *surface,
    size_t surface_size,
    DM1_V1_D0CF0108FloorOrnamentResultPc34 *out);

int run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 *out);

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
