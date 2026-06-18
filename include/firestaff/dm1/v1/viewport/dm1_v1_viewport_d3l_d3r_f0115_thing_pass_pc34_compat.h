/*
 * DM1 V1 D3L/D3R F0115 thing-pass source-lock gate.
 *
 * ReDMCSB anchors:
 *   DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
 *   lines 4547-4581, door-pass nibble strip lines 4794-4800,
 *   depth-3 view-cell clip lines 4920-4923, C10 transparent blit
 *   lines 5180-5188, creature/projectile clips lines 5211-5214 and
 *   5668-5674. F0111_DUNGEONVIEW_DrawDoor lines 4218-4337 is the
 *   door-front body between F0115 pass 1 and pass 2. F0116/F0117
 *   lines 6361-6498 and 6500-6640 are the D3L/D3R callers, reached
 *   from F0128 lines 8490-8495. DUNGEON.C F0163/F0164/F0172 lines
 *   1769-1838, 1840-1905, 2466-2523 maintain/classify thing lists.
 *   DEFS.H lines 2088, 2596-2611, 2656-2677, 4045-4046, 4139-4153,
 *   and 4228-4236 define C10, view squares, cell orders, D3L/D3R
 *   zones, cell-order zone band, and the F0115 zone bases.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0115_THING_PASS_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L_D3R_F0115_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L_D3R_F0115_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L_D3R_F0115_SOURCE_WIDTH_PC34 128
#define DM1_V1_D3L_D3R_F0115_SOURCE_HEIGHT_PC34 51
#define DM1_V1_D3L_D3R_F0115_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D3L_D3R_F0115_SIDE_D3L_PC34 = 1,
    DM1_V1_D3L_D3R_F0115_SIDE_D3R_PC34 = 2
} DM1_V1_D3LD3RF0115SidePc34;

typedef enum {
    DM1_V1_D3L_D3R_F0115_ROUTE_WALL_ALCOVE_PC34 = 0,
    DM1_V1_D3L_D3R_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34 = 1,
    DM1_V1_D3L_D3R_F0115_ROUTE_FRONT_DOOR_PASS1_PC34 = 2,
    DM1_V1_D3L_D3R_F0115_ROUTE_FRONT_DOOR_PASS2_PC34 = 3,
    DM1_V1_D3L_D3R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34 = 4
} DM1_V1_D3LD3RF0115RouteKindPc34;

typedef enum {
    DM1_V1_D3L_D3R_F0115_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D3L_D3R_F0115_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D3L_D3R_F0115_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D3L_D3R_F0115_ELEMENT_TELEPORTER_PC34 = 5,
    DM1_V1_D3L_D3R_F0115_ELEMENT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D3L_D3R_F0115_ELEMENT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D3L_D3R_F0115_ELEMENT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D3L_D3R_F0115_ELEMENT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D3LD3RF0115ElementPc34;

typedef struct {
    int route_kind;
    const char *route_name;
    int square_element;
    int source_line_first;
    int source_line_last;
    int calls_f0108_before_f0115;
    int calls_f0111_between_passes;
    int calls_f0115;
    int returns_before_f0115;
    int f0115_pass;
    unsigned int cell_order;
    int cell_count;
    int decoded_cells[4];
    int visible_cells[4];
    int visible_cell_count;
    const char *redmcsb_anchor;
} DM1_V1_D3LD3RF0115RoutePc34;

typedef struct {
    int side;
    const char *lane_name;
    int view_square_index;
    int view_depth;
    int view_lane;
    int relative_forward;
    int relative_right;
    int f0128_update_line;
    int f0128_draw_line;
    int f0128_draw_order_index;
    int f0116_or_f0117_first_line;
    int f0116_or_f0117_last_line;
    int wall_zone;
    int wall_frame_x_first;
    int wall_frame_x_last;
    int wall_frame_y_first;
    int wall_frame_y_last;
    int wall_frame_source_x_first;
    int wall_frame_source_y_first;
    int wall_frame_width;
    int wall_frame_height;
    int item_projectile_row;
    int creature_row;
    int explosion_row;
    int field_aspect_index;
    int floor_ornament_view;
    int f0172_square_aspect_count;
    int first_thing_zone_base_index;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int csb_disjoint;
    const DM1_V1_D3LD3RF0115RoutePc34 *routes;
    size_t route_table_count;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_door_anchor;
    const char *redmcsb_defs_anchor;
    const char *source_lines;
} DM1_V1_D3LD3RF0115ThingPassPc34;

typedef struct {
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture;
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
} DM1_V1_D3LD3RF0115PixelPc34;

void dm1_v1_viewport_d3l_d3r_f0115_thing_pass_init_pc34(void);

size_t dm1_v1_viewport_d3l_d3r_f0115_thing_pass_count_pc34(void);

const DM1_V1_D3LD3RF0115ThingPassPc34 *
dm1_v1_viewport_d3l_d3r_f0115_thing_pass_at_pc34(size_t index);

const DM1_V1_D3LD3RF0115ThingPassPc34 *
dm1_v1_viewport_d3l_d3r_f0115_thing_pass_for_side_pc34(int side);

const DM1_V1_D3LD3RF0115RoutePc34 *
dm1_v1_viewport_d3l_d3r_f0115_thing_pass_route_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int route_kind);

int dm1_v1_viewport_d3l_d3r_f0115_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index);

int dm1_v1_viewport_d3l_d3r_f0115_cell_visible_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l_d3r_f0115_visible_cell_at_pc34(
    const DM1_V1_D3LD3RF0115RoutePc34 *route,
    int visible_index);

int dm1_v1_viewport_d3l_d3r_f0115_cell_order_supported_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    unsigned int cell_order);

int dm1_v1_viewport_d3l_d3r_f0115_element_calls_f0115_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int element);

int dm1_v1_viewport_d3l_d3r_f0115_first_thing_zone_base_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture);

int dm1_v1_viewport_d3l_d3r_f0115_item_zone_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l_d3r_f0115_projectile_zone_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int view_cell);

int dm1_v1_viewport_d3l_d3r_f0115_creature_zone_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int coordinate_set,
    int view_cell);

int dm1_v1_viewport_d3l_d3r_f0115_explosion_zone_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int view_cell);

uint8_t dm1_v1_viewport_d3l_d3r_f0115_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3l_d3r_f0115_apply_pixel_pc34(
    const DM1_V1_D3LD3RF0115ThingPassPc34 *fixture,
    int viewport_x,
    int viewport_y,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3LD3RF0115PixelPc34 *out);

uint32_t dm1_v1_viewport_d3l_d3r_f0115_model_hash_pc34(void);

const char *dm1_v1_viewport_d3l_d3r_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0115_THING_PASS_PC34_COMPAT_H */
