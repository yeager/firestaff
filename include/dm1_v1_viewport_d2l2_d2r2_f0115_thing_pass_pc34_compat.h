/*
 * DM1 V1 D2L2/D2R2 F0115 thing-pass source-lock gate.
 * ReDMCSB anchors: DUNVIEW.C F0115 lines 4547-4581, 5180-5295, and
 * 5668-5671; DUNVIEW.C F0678/F0679 lines 6837-6896 reached from F0128
 * lines 8503-8508; DUNVIEW.C F0121 lines 7244-7388 is the adjacent D2C
 * F0115 route that must not be borrowed by D2L2/D2R2; DUNGEON.C
 * F0163/F0164/F0172 lines 1769-1838, 1840-1937, and 2466-2589 classify
 * and maintain square thing lists before viewport dispatch.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D2L2_D2R2_F0115_SIDE_D2L2_PC34 = 1,
    DM1_V1_D2L2_D2R2_F0115_SIDE_D2R2_PC34 = 2
} DM1_V1_D2L2D2R2F0115ThingPassSidePc34;

typedef struct {
    int side;
    const char *lane_name;
    int route_count;
    int f0115_call_count;
    int f0172_square_aspect_count;
    int view_square_index;
    int view_depth;
    int view_lane;
    int relative_forward;
    int relative_right;
    int f0128_draw_order_index;
    unsigned int f0115_cell_order;
    int f0115_first_cell;
    int f0115_second_cell;
    int f0115_cell_count;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int field_aspect_index;
    int wall_zone;
    int wall_case_returns_before_f0115;
    int teleporter_case_has_no_f0115;
    int corridor_case_absent;
    int door_case_absent;
    int no_f0108_contract;
    int no_f0111_contract;
    int no_f0115_contract;
    int d2c_f0115_non_interference;
    int d2c_view_square_index;
    int d2c_normal_cell_order;
    int d2c_f0128_draw_order_index;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *source_lines;
} DM1_V1_D2L2D2R2F0115ThingPassPc34;

void dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34(void);

const DM1_V1_D2L2D2R2F0115ThingPassPc34 *
dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0115ThingPassPc34 *
dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(int side);

int dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d2l2_d2r2_f0115_projectile_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d2l2_d2r2_f0115_creature_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d2l2_d2r2_f0115_explosion_zone_pc34(
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *fixture);

const char *dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0115_THING_PASS_PC34_COMPAT_H */
