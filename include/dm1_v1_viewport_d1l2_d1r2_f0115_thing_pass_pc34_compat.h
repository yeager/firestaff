#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D1L2_D1R2_F0115_SIDE_D1L2_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0115_SIDE_D1R2_PC34 = 2
} DM1_V1_D1L2D1R2F0115ThingPassSidePc34;

typedef struct {
    int side;
    const char *lane_name;
    int route_count;
    int f0115_call_count;
    int c10_transparency_flag;
    int wall_no_wall_flag;
    const char *zone_binding_tag;
    int item_zone_base;
    int projectile_zone_base;
    int creature_zone_base;
    int explosion_zone_base;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int view_square_index;
    int view_depth;
    int view_lane;
    unsigned int f0115_cell_order;
    int f0115_first_cell;
    int f0115_second_cell;
    int f0115_cell_count;
    int no_f0107_contract;
    int no_f0111_contract;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *source_lines;
} DM1_V1_D1L2D1R2F0115ThingPassPc34;

void dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(void);

const DM1_V1_D1L2D1R2F0115ThingPassPc34 *
dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D1L2D1R2F0115ThingPassPc34 *
dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(int side);

int dm1_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d1l2_d1r2_f0115_explosion_zone_pc34(
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *fixture);

const char *dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0115_THING_PASS_PC34_COMPAT_H */
