#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB-only, source-locked contract gate. This is not DM1 and does not claim
 * real-asset bitmap parity.
 *
 * ReDMCSB anchors: DUNVIEW.C:6226-6291 F0676, 6293-6358 F0677,
 * 6361-6480 F0116, 6500-6622 F0117, 4547-4581 F0115, 5668-5671
 * projectile row guard, and 8478-8508 F0128. DEFS.H:2088,2610-2611,
 * 4042-4043; COORD.C layout parent records; CSB Viewport.cpp:1903-1906.
 */

typedef enum {
    CSB_V1_D3L2_D3R2_F0115_ROUTE_PROJECTILE_PC34 = 1,
    CSB_V1_D3L2_D3R2_F0115_ROUTE_CREATURE_PC34 = 2,
    CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34 = 3,
    CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34 = 4
} CSB_V1_D3L2D3R2F0115ThingRoutePc34;

typedef struct {
    const char *contract_scope;
    const char *f0676_d3l2_lines;
    const char *f0677_d3r2_lines;
    const char *f0116_d3l_lines;
    const char *f0117_d3r_lines;
    const char *f0115_order_lines;
    const char *f0115_projectile_guard_lines;
    const char *f0128_dispatch_lines;
    const char *defs_lines;
    const char *coord_parent_record_lines;
    const char *csb_lineage_thing_pass_lines;
    const char *csb_lineage_overlay_lines;
} CSB_V1_D3L2D3R2F0115ThingPassEvidencePc34;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int view_square;
    int route;
    const char *route_name;
    int spec_table_index;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int view_depth;
    int view_lane;
    int wall_zone;
    int field_aspect_index;
    int f0676_f0677_has_f0115_route;
    unsigned int open_cell_order;
    unsigned int door_front_rear_f0115_order;
    unsigned int door_front_f0111_order;
    unsigned int door_front_front_f0115_order;
    int no_door_panel_marker;
    int f0111_excluded_from_thing_pass;
    int f0116_f0117_inner_square_non_owner;
    int f0128_left_then_right_dispatch;
    int thing_pass_route_enabled;
    int g2028_row;
    int g2033_row;
    int g2034_row;
    int projectile_row_selection_uses_g2028;
    int projectile_row_guard_5668_5671;
    int projectile_restarts_thing_list;
    int projectile_requires_type_c14;
    int projectile_requires_cell_match;
    int projectile_suppresses_depth3_front_cells;
    int projectile_suppresses_depth0_back_cells;
    int projectile_zone_base;
    int projectile_zone_cell_stride;
    int item_requires_weapon_to_junk;
    int item_requires_cell_match;
    int item_suppresses_depth3_front_cells;
    int item_suppresses_depth0_back_cells;
    int item_zone_base;
    int item_zone_cell_stride;
    int item_shift_mask;
    int item_pile_shift_advances;
    int creature_requires_group_marker;
    int creature_zone_base;
    int creature_coordinate_set_stride;
    int creature_zone_cell_stride;
    int creature_shift_mask;
    int explosion_restarts_thing_list_after_cells;
    int explosion_restart_uses_g2034_g2035;
    int explosion_rebirth_step1_zone_base;
    int explosion_rebirth_step2_zone_base;
    int explosion_centered_zone_base;
    int explosion_side_zone_base;
    int explosion_side_zone_cell_stride;
    int fluxcage_defers_to_f0113;
    int fluxcage_field_zone;
    int transparent_color;
    int c10_transparency_skip;
    int derived_bitmap_none;
    int scaled_path_uses_cm1_derived_bitmap_none;
    int dynamic_horizontal_flip_flag;
    int dynamic_vertical_flip_flag;
    int coord_item_parent_record;
    int coord_explosion_parent_record;
    int coord_creature_parent_record;
    int csb_lineage_relative_cell;
    int csb_lineage_contents_opcode;
    int csb_lineage_xy_opcode;
    int csb_lineage_draw_order_opcode;
    int csb_lineage_std_draw_room_objects_opcode;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_D3L2D3R2F0115ThingPassSpecPc34;

int csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34(void);

size_t csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(void);

const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(size_t index);

const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
    int view_square,
    int route);

const CSB_V1_D3L2D3R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_evidence_pc34(void);

int csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell);

int csb_v1_viewport_d3l2_d3r2_f0115_item_layout_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell);

int csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell);

int csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int coordinate_set,
    int view_cell);

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step1_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec);

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step2_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec);

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_centered_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec);

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell);

int csb_v1_viewport_d3l2_d3r2_f0115_apply_c10_blit_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal,
    int flip_vertical);

const char *csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
