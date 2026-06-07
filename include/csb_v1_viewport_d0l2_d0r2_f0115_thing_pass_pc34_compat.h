#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L lines
 * 7960-8062 and F0126_DUNGEONVIEW_DrawSquareD0R lines 8064-8162;
 * F0128 D0 side dispatch at 8536-8541; F0115 lines 4547-4581,
 * 4806-4811, 4923, 5201-5214, 5295, 5615-5617, 5668-5683,
 * 5916-5923, 5998-5999, 6107, and 6122; F0674_F0128_sub floor/
 * ceiling bitmap copy at 2995-3015; G0163 per-frame wall bitmap rows at
 * 581-594. CSB-lineage cross-reference: Viewport.cpp:1192-1209 open
 * F0L1/F0R1 routes and 1903-1915 center door-facing dispatch.
 */

typedef enum {
    CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34 = 2
} CSB_V1_D0L2D0R2F0115ThingPassSidePc34;

typedef struct {
    const char *contract_scope;
    const char *f0125_d0l_lines;
    const char *f0126_d0r_lines;
    const char *f0128_dispatch_lines;
    const char *f0115_lines;
    const char *f0674_lines;
    const char *frame_lines;
    const char *defs_lines;
    const char *csb_lineage_open_lines;
    const char *csb_lineage_center_door_lines;
} CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34;

typedef struct {
    int side;
    const char *lane_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int route_count;
    int f0115_call_count;
    int view_square_index;
    int view_depth;
    int view_lane;
    unsigned int f0115_cell_order;
    int f0115_first_cell;
    int f0115_cell_count;
    int wall_frame_row;
    int wall_frame_x1;
    int wall_frame_x2;
    int wall_frame_y1;
    int wall_frame_y2;
    int wall_frame_byte_width;
    int wall_frame_height;
    int wall_frame_source_x;
    int wall_frame_source_y;
    int viewport_clip_x1;
    int viewport_clip_x2;
    int viewport_clip_y1;
    int viewport_clip_y2;
    int source_clip_y1;
    int source_clip_y2;
    int c10_transparency_flag;
    int transparent_color;
    int no_write_on_transparent;
    int no_write_outside_viewport_clip;
    int no_write_outside_source_y_clip;
    int item_projectile_row;
    int item_projectile_disabled_by_g2028;
    int creature_row;
    int creature_cell_gate;
    int explosion_row;
    int field_aspect_index;
    int wall_zone;
    int ceiling_pit_zone;
    int f0112_before_f0115;
    int teleporter_field_after_f0115;
    int wall_route_excluded;
    int no_f0107_contract;
    int no_f0111_contract;
    int no_custom_backgrounds_contract;
    int f0115_draw_order_objects_first;
    int f0115_draw_order_creatures_second;
    int f0115_draw_order_projectiles_third;
    int f0115_draw_order_explosions_last;
    int f0674_per_frame_bitmap_copy;
    int wall_set_frame_used_for_field;
    int wall_set_draw_order_d0l_before_d0r;
    int csb_lineage_relative_cell;
    int csb_lineage_contents_opcode;
    int csb_lineage_draw_order_opcode;
    int csb_lineage_std_draw_room_objects_opcode;
    int csb_lineage_center_door_draw_order_first;
    int csb_lineage_center_door_draw_order_second;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *source_lines;
} CSB_V1_D0L2D0R2F0115ThingPassPc34;

int csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void);

size_t csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void);

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index);

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side);

const CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_evidence_pc34(void);

int csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y);

int csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int source_y);

unsigned char csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    unsigned char destination,
    unsigned char source);

int csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y,
    int source_y,
    unsigned char source,
    unsigned char *destination);

int csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

int csb_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture);

int csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell);

const char *csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
