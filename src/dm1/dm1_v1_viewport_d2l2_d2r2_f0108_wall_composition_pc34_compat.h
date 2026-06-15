#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_WALL_COMPOSITION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L2_D2R2_F0108_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34 = 0,
    DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34 = 1
} DM1_V1_D2L2D2R2F0108WallSidePc34;

typedef enum {
    DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34 = 0,
    DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34 = 1
} DM1_V1_D2L2D2R2F0108WallContextPc34;

typedef struct {
    DM1_V1_D2L2D2R2F0108WallSidePc34 side;
    DM1_V1_D2L2D2R2F0108WallContextPc34 context;
    const char *label;
    int draw_order_index;
    int relative_depth;
    int relative_lateral;
    int requested_guard_view_square;
    int f0108_owner_view_square;
    int f0108_floor_view;
    int f0108_square_aspect_slot;
    int f0108_floor_zone_base;
    int f0108_floor_zone_stride_pc34;
    bool f0108_right_side_flips;
    bool footprints_recurse_same_view;
    bool wall_element_excludes_f0108;
    bool d2l2_d2r2_guard_excludes_f0108;
    bool calls_ceiling_pit_after_f0108;
    bool calls_f0111_after_f0108;
    int f0111_door_zone;
    int f0111_door_frame_top_zone;
    unsigned int f0115_open_cell_order;
    unsigned int f0115_door_pass1_cell_order;
    unsigned int f0115_door_pass2_cell_order;
    int transparent_color;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D2L2D2R2F0108WallSpecPc34;

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
} DM1_V1_D2L2D2R2F0108WallOrdinalPc34;

typedef struct {
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *spec;
    uint8_t destination_before;
    uint8_t floor_pixel;
    uint8_t after_floor;
    uint8_t ceiling_or_frame_pixel;
    uint8_t after_ceiling_or_frame;
    uint8_t pass1_pixel;
    uint8_t after_pass1;
    uint8_t door_pixel;
    uint8_t after_door;
    uint8_t pass2_pixel;
    uint8_t after_pass2;
    bool floor_transparent;
    bool ceiling_or_frame_transparent;
    bool pass1_transparent;
    bool door_transparent;
    bool pass2_transparent;
    bool door_front_sequence;
} DM1_V1_D2L2D2R2F0108WallPixelPc34;

size_t dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_count_pc34(void);

const DM1_V1_D2L2D2R2F0108WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0108WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
    DM1_V1_D2L2D2R2F0108WallSidePc34 side,
    DM1_V1_D2L2D2R2F0108WallContextPc34 context);

bool dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2L2D2R2F0108WallOrdinalPc34 *out);

uint8_t dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *spec,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t ceiling_or_frame_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D2L2D2R2F0108WallPixelPc34 *out);

const char *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
