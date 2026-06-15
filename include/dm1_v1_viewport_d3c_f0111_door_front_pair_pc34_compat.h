#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_F0111_DOOR_FRONT_PAIR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_F0111_DOOR_FRONT_PAIR_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3C_F0111_DOOR_FRONT_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3C_F0111_DOOR_FRONT_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3C_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3C_F0111_DOOR_FRONT_C45_PRESERVED_PIXEL_PC34 45

typedef enum {
    DM1_V1_D3C_F0111_STEP_F0108_FLOOR_ORNAMENT_PC34 = 0,
    DM1_V1_D3C_F0111_STEP_F0115_PASS1_PC34,
    DM1_V1_D3C_F0111_STEP_F0104_LEFT_FRAME_PC34,
    DM1_V1_D3C_F0111_STEP_F0105_RIGHT_FRAME_FLIPPED_PC34,
    DM1_V1_D3C_F0111_STEP_F0111_DOOR_FRONT_PC34,
    DM1_V1_D3C_F0111_STEP_F0115_PASS2_PC34
} DM1_V1_D3CF0111DoorFrontStepPc34;

typedef struct {
    int side;
    const char *label;
    int draw_order_index;
    int view_square_index;
    int relative_depth;
    int relative_lateral;
    int wall_zone;
    int wall_ornament_view;
    int floor_ornament_view;
    int floor_ornament_slot;
    int front_wall_ornament_slot;
    int door_zone;
    int door_zone_d3l_neighbor;
    int door_zone_d3r_neighbor;
    int door_zone_d2c_disjoint;
    int door_zone_d2r_disjoint;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int door_frame_bitmap_id;
    int front_bitmap_id;
    const char *front_bitmap_symbol;
    int door_ornament_view;
    unsigned int pass1_cell_order;
    unsigned int pass2_cell_order;
    int door_frame_x_first;
    int door_frame_x_last;
    int door_frame_y_first;
    int door_frame_y_last;
    int door_frame_byte_width;
    int door_frame_height;
    int door_source_x_first;
    int door_source_y_first;
    int transparent_color;
    int c45_preserved_pixel;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    const char *redmcsb_f0104_anchor;
    const char *redmcsb_f0105_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D3CF0111DoorFrontSpecPc34;

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_D3CF0111DoorFrontRectPc34;

typedef struct {
    int wall_ornament_ordinal;
    int wall_ornament_view;
    int native_bitmap_index;
    int coordinate_set;
    int zone_index;
    int palette_replacement_index_c10;
    int transparent_color;
    DM1_V1_D3CF0111DoorFrontRectPc34 draw_rect;
    DM1_V1_D3CF0111DoorFrontRectPc34 keepout_rect;
    bool draw_allowed_in_wall_case;
    bool rejected_in_door_front_case;
    const char *redmcsb_anchor;
} DM1_V1_D3CF0111WallOrnamentPc34;

typedef struct {
    const DM1_V1_D3CF0111DoorFrontSpecPc34 *spec;
    bool in_frame;
    bool outside_untouched;
    bool floor_transparent;
    bool pass1_transparent;
    bool left_frame_transparent;
    bool right_frame_transparent;
    bool door_transparent;
    bool pass2_transparent;
    int viewport_x;
    int viewport_y;
    uint8_t destination_before;
    uint8_t floor_pixel;
    uint8_t after_floor;
    uint8_t pass1_pixel;
    uint8_t after_pass1;
    uint8_t left_frame_pixel;
    uint8_t after_left_frame;
    uint8_t right_frame_pixel;
    uint8_t after_right_frame;
    uint8_t door_pixel;
    uint8_t after_door;
    uint8_t pass2_pixel;
    uint8_t after_pass2;
} DM1_V1_D3CF0111DoorFrontPixelPc34;

typedef struct {
    DM1_V1_D3CF0111DoorFrontStepPc34 step;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D3CF0111DoorFrontStepInfoPc34;

size_t dm1_v1_viewport_d3c_f0111_door_front_pair_count_pc34(void);

const DM1_V1_D3CF0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3c_f0111_door_front_pair_at_pc34(size_t index);

const DM1_V1_D3CF0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3c_f0111_door_front_pair_center_pc34(void);

size_t dm1_v1_viewport_d3c_f0111_door_front_pair_steps_pc34(
    DM1_V1_D3CF0111DoorFrontStepInfoPc34 *out,
    size_t cap);

const DM1_V1_D3CF0111WallOrnamentPc34 *
dm1_v1_viewport_d3c_f0111_wall_ornament_keepout_pc34(void);

bool dm1_v1_viewport_d3c_f0111_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

bool dm1_v1_viewport_d3c_f0111_rects_overlap_pc34(
    DM1_V1_D3CF0111DoorFrontRectPc34 a,
    DM1_V1_D3CF0111DoorFrontRectPc34 b);

int dm1_v1_viewport_d3c_f0111_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index);

uint8_t dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
    const DM1_V1_D3CF0111DoorFrontSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t left_frame_pixel,
    uint8_t right_frame_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D3CF0111DoorFrontPixelPc34 *out);

uint32_t dm1_v1_viewport_d3c_f0111_door_front_pair_hash_pc34(void);

const char *
dm1_v1_viewport_d3c_f0111_door_front_pair_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
