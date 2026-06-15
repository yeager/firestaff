#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0111_DOOR_FRONT_PAIR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0111_DOOR_FRONT_PAIR_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L2_D2R2_F0111_DOOR_FRONT_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L2_D2R2_F0111_DOOR_FRONT_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L2_D2R2_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D2L2_D2R2_F0111_SIDE_D2L_PC34 = 1,
    DM1_V1_D2L2_D2R2_F0111_SIDE_D2R_PC34 = 2
} DM1_V1_D2L2D2R2F0111DoorFrontSidePc34;

typedef struct {
    int side;
    const char *label;
    int draw_order_index;
    int requested_view_square_index;
    int f0111_view_square_index;
    int requested_square_has_f0111_route;
    int relative_depth;
    int relative_lateral;
    int f0107_side_wall_view;
    int f0107_front_wall_view;
    int f0108_floor_view;
    int f0108_ornament_aspect_slot;
    int f0111_door_zone;
    int f0111_door_frame_top_zone;
    int f0111_front_bitmap_id;
    const char *f0111_front_bitmap_symbol;
    int f0111_door_ornament_view;
    unsigned int f0115_pass1_cell_order;
    unsigned int f0115_pass2_cell_order;
    int door_frame_x_first;
    int door_frame_x_last;
    int door_frame_y_first;
    int door_frame_y_last;
    int door_frame_byte_width;
    int door_frame_height;
    int door_source_x_first;
    int door_source_y_first;
    int transparent_color;
    int d2l2_no_f0111_route_square_index;
    int d2r2_no_f0111_route_square_index;
    int d2l2_no_f0107_f0108_f0111_route;
    int d2r2_no_f0107_f0108_f0111_route;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_dispatch_anchor;
} DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34;

typedef struct {
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *spec;
    bool in_clip;
    bool no_write_metadata;
    bool floor_transparent;
    bool pass1_transparent;
    bool door_transparent;
    bool pass2_transparent;
    int viewport_x;
    int viewport_y;
    uint8_t destination_before;
    uint8_t floor_pixel;
    uint8_t after_floor;
    uint8_t pass1_pixel;
    uint8_t after_pass1;
    uint8_t door_pixel;
    uint8_t after_door;
    uint8_t pass2_pixel;
    uint8_t after_pass2;
} DM1_V1_D2L2D2R2F0111DoorFrontPixelPc34;

size_t dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_count_pc34(void);

const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(int side);

bool dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index);

uint8_t dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D2L2D2R2F0111DoorFrontPixelPc34 *out);

const char *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_source_evidence_pc34(void);

extern const char
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_csb_lineage_evidence_pc34[];

#ifdef __cplusplus
}
#endif

#endif
