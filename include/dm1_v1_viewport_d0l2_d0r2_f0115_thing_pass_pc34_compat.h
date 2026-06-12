#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

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
    int f0108_floor_stage_before_f0115;
    int f0108_direct_call_count;
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
    int fluxcage_field_zone;
    int fluxcage_field_after_explosions;
    int fluxcage_suppressed_on_door_pass1;
    int fluxcage_suppressed_during_endgame;
    int door_zone;
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
    int f0163_not_called_by_draw;
    int f0164_not_called_by_draw;
    int f0172_square_aspect_source;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
    const char *source_lines;
} DM1_V1_D0L2D0R2F0115ThingPassPc34;

typedef struct {
    int ok;
    int f0108_calls;
    int f0115_calls;
    uint8_t after_f0108;
    uint8_t after_f0115;
    int f0108_transparent;
    int f0115_transparent;
} DM1_V1_D0L2D0R2F0115ThingPassTracePc34;

void dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34(void);

size_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(int side);

const DM1_V1_D0L2D0R2F0115ThingPassPc34 *
dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side);

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
    unsigned int order,
    int ordinal);

uint8_t dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *spec,
    uint8_t base_pixel,
    uint8_t f0108_floor_pixel,
    uint8_t f0115_thing_pixel,
    DM1_V1_D0L2D0R2F0115ThingPassTracePc34 *out_trace);

int dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_is_draw_mutating_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *spec);

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

int dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int door_front_pass,
    int endgame_suppressed);

const char *dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H */
