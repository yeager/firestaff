#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D1L_D1R_F0115_LANE_D1L_PC34 = 0,
    DM1_V1_D1L_D1R_F0115_LANE_D1R_PC34 = 1
} DM1V1D1LD1RF0115LanePc34;

typedef enum {
    DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS1_PC34 = 0,
    DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS2_PC34 = 1,
    DM1_V1_D1L_D1R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34 = 2
} DM1V1D1LD1RF0115RouteKindPc34;

typedef struct {
    int source_named_cell;
    int ordinal_low_to_high;
    int decoded_view_cell_index;
    int kept_by_depth1_clip;
    int first_thing_square_aspect_slot;
} DM1V1D1LD1RF0115CellPc34;

typedef struct {
    int route_kind;
    const char *route_name;
    int caller_line;
    int f0111_door_line;
    int order_assign_line;
    int raw_cell_order;
    int door_front_marker_nibble;
    int door_front_pass;
    int stripped_cell_order;
    int required_for_corridor;
    int required_for_pit;
    int required_for_teleporter;
    size_t source_named_cell_count;
    int source_named_cells[2];
    size_t decoded_cell_count;
    int decoded_cells[2];
    const char *order_symbol;
    const char *redmcsb_anchor;
} DM1V1D1LD1RF0115RoutePc34;

typedef struct {
    int lane;
    const char *lane_name;
    int relative_depth;
    int relative_lateral;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0128_update_line;
    int f0128_dispatch_line;
    int view_square;
    int view_lane;
    int view_depth;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int door_zone;
    int c10_transparent_color;
    int m550_first_thing_square_aspect_slot;
    int m550_media020_square_aspect_slot;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    size_t route_count;
    DM1V1D1LD1RF0115RoutePc34 routes[3];
    DM1V1D1LD1RF0115CellPc34 depth1_cells[4];
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
} DM1V1D1LD1RF0115LanePc34Data;

void dm1_v1_viewport_d1l_d1r_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(void);

const DM1V1D1LD1RF0115LanePc34Data *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(size_t index);

const DM1V1D1LD1RF0115LanePc34Data *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(
    int view_square);

const DM1V1D1LD1RF0115RoutePc34 *
dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int route_kind);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(
    int element_type);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(
    int element_type);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(
    int view_cell);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(
    int view_cell);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int raw_cell_order);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_door_pass_from_order_pc34(
    int raw_cell_order);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_strip_door_order_pc34(
    int raw_cell_order);

int dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(
    const DM1V1D1LD1RF0115LanePc34Data *lane,
    int view_cell);

uint32_t dm1_v1_viewport_d1l_d1r_f0115_thing_pass_hash_pc34(void);

const char *dm1_v1_viewport_d1l_d1r_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
