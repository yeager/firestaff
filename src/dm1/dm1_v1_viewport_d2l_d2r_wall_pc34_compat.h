#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34 = 0,
    DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34 = 1
} DM1_V1_D2LD2RWallSidePc34;

typedef struct {
    DM1_V1_D2LD2RWallSidePc34 side;
    int row;
    int viewport_x;
    uint8_t transparent_color;
} DM1_V1_D2LD2RWallPixelInputPc34;

typedef struct {
    DM1_V1_D2LD2RWallSidePc34 side;
    bool contract_only;
    bool real_asset_bitmap_parity;
    int depth;
    int lateral;
    int view_square_index;
    int native_wall_index_pc34;
    int flipped_wall_index_pc34;
    int wall_zone_pc34;
    int draw_order_index;
    int center_field_order_index;
    int raw_frame_viewport_x_first;
    int raw_frame_viewport_x_last;
    int raw_frame_viewport_y_first;
    int raw_frame_viewport_y_last;
    int raw_frame_byte_width;
    int raw_frame_height;
    int raw_frame_source_x;
    int raw_frame_source_y;
    int viewport_x_first;
    int viewport_x_last;
    int viewport_y_first;
    int viewport_y_last;
    int source_x_first;
    int source_x_last;
    int source_y_first;
    int source_y_last;
    int source_width;
    int source_height;
    int visible_width;
    int visible_height;
    bool uses_f0100_frame_blit;
    bool uses_f0105_party_side_flip;
    bool uses_g0699_wall_d2lcr_pointer;
    bool preserves_c10_transparency;
    bool wall_case_returns;
    bool calls_f0107_side_ornament_probe;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    const char *view_square_symbol;
    const char *native_wall_symbol;
    const char *flipped_wall_symbol;
    const char *wall_zone_symbol;
    const char *source_lines;
} DM1_V1_D2LD2RWallSpecPc34;

typedef struct {
    DM1_V1_D2LD2RWallSpecPc34 spec;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_D2LD2RWallPixelResultPc34;

#define DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 234
#define DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 72
#define DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 144
#define DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34 71
#define DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34 10

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2LD2RWallSidePc34 side);

bool dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    int *source_x,
    int *source_y);

bool dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    bool party_side_flipped,
    int *source_x,
    int *source_y);

bool dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
    const DM1_V1_D2LD2RWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2LD2RWallPixelResultPc34 *out);

uint8_t dm1_v1_viewport_d2l_d2r_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H */
