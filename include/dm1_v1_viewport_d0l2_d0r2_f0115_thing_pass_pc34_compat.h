#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34 = 2
} DM1_V1_D0L2D0R2F0115ThingPassSidePc34;

typedef struct {
    int side;
    const char *lane_name;
    int route_count;
    int f0115_call_count;
    int view_square_index;
    int view_depth;
    int view_lane;
    unsigned int f0115_cell_order;
    int f0115_first_cell;
    int f0115_cell_count;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int field_aspect_index;
    int wall_zone;
    int ceiling_pit_zone;
    int field_draw_after_f0115;
    int item_projectile_disabled_by_g2028;
    int quarter_creature_cell_gate;
    int f0112_before_f0115;
    int no_f0107_contract;
    int no_f0111_contract;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *source_lines;
} DM1_V1_D0L2D0R2F0115ThingPassPc34;

void dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side);

int dm1_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture);

int dm1_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

const char *dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H */
