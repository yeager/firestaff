#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_SIDEWALL_PAIR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_SIDEWALL_PAIR_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L_D3R_SIDEWALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L_D3R_SIDEWALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L_D3R_SIDEWALL_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D3L_D3R_SIDEWALL_SIDE_D3L_PC34 = 1,
    DM1_V1_D3L_D3R_SIDEWALL_SIDE_D3R_PC34 = 2
} DM1_V1_D3LD3RSidewallSidePc34;

typedef struct {
    int side;
    const char *side_name;
    int draw_order_index;
    int view_square_index;
    int relative_depth;
    int relative_lateral;
    int wall_zone;
    int wall_bitmap;
    int flipped_wall_bitmap;
    int side_wall_ornament_view;
    int front_wall_ornament_view;
    int center_front_view;
    int door_zone;
    int door_front_bitmap_id;
    const char *door_front_bitmap_symbol;
    int door_ornament_view;
    int f0115_door_pass1_cell_order;
    int f0115_door_pass2_cell_order;
    int f0115_alcove_cell_order;
    int f0115_view_square;
    int thing_item_row;
    int creature_row;
    int explosion_row;
    int frame_x_first;
    int frame_x_last;
    int frame_y_first;
    int frame_y_last;
    int source_x_first;
    int source_y_first;
    int transparent_color;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    const char *redmcsb_wall_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0115_anchor;
} DM1_V1_D3LD3RSidewallSpecPc34;

typedef struct {
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec;
    bool in_clip;
    bool no_write_metadata;
    bool wall_transparent;
    bool ornament_transparent;
    bool door_transparent;
    bool thing_transparent;
    int viewport_x;
    int viewport_y;
    uint8_t destination_before;
    uint8_t wall_pixel;
    uint8_t after_wall;
    uint8_t ornament_pixel;
    uint8_t after_ornament;
    uint8_t door_pixel;
    uint8_t after_door;
    uint8_t thing_pixel;
    uint8_t after_thing;
} DM1_V1_D3LD3RSidewallPixelPc34;

size_t dm1_v1_viewport_d3l_d3r_sidewall_pair_count_pc34(void);

const DM1_V1_D3LD3RSidewallSpecPc34 *
dm1_v1_viewport_d3l_d3r_sidewall_pair_at_pc34(size_t index);

const DM1_V1_D3LD3RSidewallSpecPc34 *
dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(int side);

bool dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_door_zone_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec);

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_front_bitmap_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec);

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0115_view_square_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec);

int dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index);

uint8_t dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t wall_pixel,
    uint8_t ornament_pixel,
    uint8_t door_pixel,
    uint8_t thing_pixel,
    DM1_V1_D3LD3RSidewallPixelPc34 *out);

const char *
dm1_v1_viewport_d3l_d3r_sidewall_pair_source_evidence_pc34(void);

extern const char
dm1_v1_viewport_d3l_d3r_sidewall_pair_csb_lineage_viewport_cpp_evidence_pc34[];

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_SIDEWALL_PAIR_PC34_COMPAT_H */
