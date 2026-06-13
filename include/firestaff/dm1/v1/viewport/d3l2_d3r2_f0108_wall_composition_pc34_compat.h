#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34 16
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34 16
#define DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34 3

typedef enum {
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_D3R2_PC34 = 1,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_COUNT_PC34 = 2
} DM1V1D3L2D3R2F0108WallCompositionSidePc34;

typedef enum {
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_TELEPORTER_PC34 = 5,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_STAIRS_FRONT_PC34 = 19
} DM1V1D3L2D3R2F0108WallCompositionElementPc34;

typedef struct {
    DM1V1D3L2D3R2F0108WallCompositionSidePc34 side;
    const char *side_name;
    const char *function_name;
    int row_depth;
    int relative_lateral;
    int f0128_row_order;
    int f0128_update_line;
    int f0128_draw_line;
    int function_start_line;
    int function_end_line;
    int view_square;
    int view_floor;
    int wall_zone;
    int wall_view;
    int wall_aspect_slot;
    int floor_aspect_slot;
    int door_state_slot;
    int door_thing_slot;
    int door_zone;
    unsigned int open_cell_order;
    unsigned int side_cell_order;
    unsigned int door_pass1_cell_order;
    unsigned int door_pass2_cell_order;
    int f0108_open_line;
    int f0108_door_front_line;
    int f0115_pass1_line;
    int f0111_line;
    int f0115_pass2_line;
    bool right_side_floor_flips;
    bool wall_case_returns_before_f0108;
    bool door_front_uses_two_pass_order;
    bool f0111_transparency_contract_only;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    const char *redmcsb_f067x_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
} DM1V1D3L2D3R2F0108WallCompositionSpecPc34;

typedef struct {
    DM1V1D3L2D3R2F0108WallCompositionSidePc34 side;
    DM1V1D3L2D3R2F0108WallCompositionElementPc34 element;
    const char *element_name;
    int route_index;
    int route_start_line;
    int route_end_line;
    bool supported_by_f067x;
    bool calls_wall_f0107;
    bool calls_f0108;
    bool calls_f0115_pass1;
    bool calls_f0111;
    bool calls_f0115_pass2;
    bool returns_after_wall;
    bool field_tail_after_teleporter;
    bool d3c_f0107_keepout;
    bool d0l2_d0r2_f0107_keepout;
    unsigned int cell_order;
    const char *redmcsb_anchor;
} DM1V1D3L2D3R2F0108WallCompositionRoutePc34;

typedef struct {
    unsigned int input_ordinal;
    bool has_input_ordinal;
    bool footprint_flag_set;
    unsigned int cleared_ordinal;
    bool primary_draws;
    unsigned int primary_ordinal;
    int primary_index;
    bool recursive_footprints_draw;
    unsigned int recursive_footprints_ordinal;
    int recursive_footprints_index;
} DM1V1D3L2D3R2F0108WallCompositionOrdinalPc34;

typedef struct {
    int pixel_index;
    DM1V1D3L2D3R2F0108WallCompositionSidePc34 side;
    DM1V1D3L2D3R2F0108WallCompositionElementPc34 element;
    uint8_t before;
    uint8_t f0108_source;
    uint8_t pass1_source;
    uint8_t f0111_source;
    uint8_t pass2_source;
    uint8_t after_f0108;
    uint8_t after_pass1;
    uint8_t after_f0111;
    uint8_t after_pass2;
    bool f0108_transparent;
    bool pass1_transparent;
    bool f0111_transparent;
    bool pass2_transparent;
    bool door_front_sequence;
    const char *redmcsb_anchor;
} DM1V1D3L2D3R2F0108WallCompositionPixelPc34;

typedef struct {
    const char *name;
    int relative_depth;
    int relative_lateral;
    int wall_zone_first;
    int wall_zone_last;
    int view_square_first;
    int view_square_last;
    int f0107_owner_commit_pinned;
    const char *why_disjoint;
} DM1V1D3L2D3R2F0108WallCompositionRejectedPc34;

typedef struct {
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int c10_transparent_color;
    int d3_row_depth;
    int d3l2_f0128_order;
    int d3r2_f0128_order;
    int d3l_f0128_order;
    int d3r_f0128_order;
    int d3c_f0128_order;
    int d2l2_f0128_order;
    int d2r2_f0128_order;
    int d3c_f0107_pass777_commit_pinned;
    int d0l2_d0r2_f0107_cc6b81b59_commit_pinned;
    int door_front_view_drawing_pass_first;
    int door_front_view_drawing_pass_second;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    uint64_t deterministic_hash;
    DM1V1D3L2D3R2F0108WallCompositionSpecPc34 specs[DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_SIDE_COUNT_PC34];
    DM1V1D3L2D3R2F0108WallCompositionRoutePc34 routes[DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34];
    DM1V1D3L2D3R2F0108WallCompositionPixelPc34 pixels[DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34];
    DM1V1D3L2D3R2F0108WallCompositionRejectedPc34 rejected[DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34];
    const char *source_evidence;
    const char *disjointness_note;
} DM1V1D3L2D3R2F0108WallCompositionModelPc34;

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1V1D3L2D3R2F0108WallCompositionOrdinalPc34 *out);

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_case_pc34(
    DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *pixel);

bool dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(
    DM1V1D3L2D3R2F0108WallCompositionModelPc34 *out_model);

uint64_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_hash_model_pc34(
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model);

const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34(void);

uint64_t dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_deterministic_hash_pc34(void);

const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_spec_at_pc34(size_t index);

const DM1V1D3L2D3R2F0108WallCompositionRoutePc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_route_at_pc34(size_t index);

const DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_at_pc34(size_t index);

const DM1V1D3L2D3R2F0108WallCompositionRejectedPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_rejected_at_pc34(size_t index);

const char *dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_source_evidence_pc34(void);
const char *dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
