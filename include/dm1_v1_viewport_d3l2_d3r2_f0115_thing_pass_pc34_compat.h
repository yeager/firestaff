/*
 * DM1 V1 D3L2/D3R2 F0115 thing-pass source-lock gate.
 * ReDMCSB anchors: DUNVIEW.C F0674_F0128_sub prototype line 1943;
 * F0115 lines 4547-4581, 4923, 5180-5188, 5211-5214, 5668-5675,
 * and 5920-5923; F0676/F0677 lines 6235-6290 and 6293-6357 reached
 * from F0128 lines 8478-8486; G0711/G0712 per-frame wall bitmaps
 * lines 579-580; DUNGEON.C F0163/F0164/F0172 lines 1769-1838,
 * 1840-1937, and 2466-2589 maintain/classify the square thing list.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L2_D3R2_F0115_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L2_D3R2_F0115_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34 16
#define DM1_V1_D3L2_D3R2_F0115_SOURCE_HEIGHT_PC34 49
#define DM1_V1_D3L2_D3R2_F0115_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D3L2_D3R2_F0115_SIDE_D3L2_PC34 = 1,
    DM1_V1_D3L2_D3R2_F0115_SIDE_D3R2_PC34 = 2
} DM1_V1_D3L2D3R2F0115ThingPassSidePc34;

typedef enum {
    DM1_V1_D3L2_D3R2_F0115_ROUTE_WALL_PC34 = 0,
    DM1_V1_D3L2_D3R2_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34 = 1,
    DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS1_PC34 = 2,
    DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS2_PC34 = 3,
    DM1_V1_D3L2_D3R2_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34 = 4
} DM1_V1_D3L2D3R2F0115RouteKindPc34;

typedef struct {
    int route_kind;
    const char *route_name;
    int square_element;
    int calls_f0108_before_f0115;
    int calls_f0111_between_passes;
    int calls_f0115;
    int f0115_pass;
    unsigned int cell_order;
    int cell_count;
    int first_view_cell;
    int second_view_cell;
    int third_view_cell;
    int fourth_view_cell;
    const char *redmcsb_anchor;
} DM1_V1_D3L2D3R2F0115RoutePc34;

typedef struct {
    int side;
    const char *lane_name;
    int route_count;
    int f0115_call_routes;
    int f0172_square_aspect_count;
    int view_square_index;
    int view_depth;
    int view_lane;
    int relative_forward;
    int relative_right;
    int f0128_draw_order_index;
    int wall_prepass_draw_order_index;
    int frame_viewport_x_first;
    int frame_viewport_x_last;
    int frame_viewport_y_first;
    int frame_viewport_y_last;
    int frame_source_x_first;
    int frame_source_y_first;
    int frame_width;
    int frame_height;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int field_aspect_index;
    int wall_zone;
    int wall_case_returns_before_f0115;
    int wall_case_calls_f0107;
    int side_route_uses_f0108;
    int front_door_two_f0115_passes;
    int corridor_route_uses_f0108;
    int teleporter_field_after_f0115;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    const DM1_V1_D3L2D3R2F0115RoutePc34 *routes;
    size_t route_table_count;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_bitmap_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *source_lines;
} DM1_V1_D3L2D3R2F0115ThingPassPc34;

typedef struct {
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture;
    bool in_viewport_clip;
    bool source_y_clipped;
    bool transparent_skip;
    bool writes_pixel;
    bool no_write_metadata;
    int viewport_x;
    int viewport_y;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t destination_before;
    uint8_t source_pixel;
    uint8_t destination_after;
} DM1_V1_D3L2D3R2F0115PixelPc34;

void dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(void);

const DM1_V1_D3L2D3R2F0115ThingPassPc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D3L2D3R2F0115ThingPassPc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(int side);

const DM1_V1_D3L2D3R2F0115RoutePc34 *
dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_route_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int route_kind);

int dm1_v1_viewport_d3l2_d3r2_f0115_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index);

int dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int coordinate_set,
    int view_cell);

int dm1_v1_viewport_d3l2_d3r2_f0115_explosion_zone_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int view_cell);

uint8_t dm1_v1_viewport_d3l2_d3r2_f0115_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *fixture,
    int viewport_x,
    int viewport_y,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3L2D3R2F0115PixelPc34 *out);

const char *dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0115_THING_PASS_PC34_COMPAT_H */
