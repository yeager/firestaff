#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D3C_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D3C_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D3C_ELEMENT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D3C_ELEMENT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D3C_ELEMENT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D3CWallElementPc34;

typedef struct {
    DM1_V1_D3CWallElementPc34 element;
    int row;
    int viewport_x;
    uint8_t transparent_color;
    bool f0107_alcove_result;
} DM1_V1_D3CWallPixelInputPc34;

typedef struct {
    bool contract_only;
    bool real_asset_pixel_parity;
    int view_square_index;
    int frame_index;
    int frame_x1;
    int frame_x2;
    int frame_y1;
    int frame_y2;
    int frame_table_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
    int f0100_source_byte_width;
    int f0100_viewport_byte_width;
    int transparent_color;
    int source_width;
    int source_height;
    int source_x_first;
    int source_x_last;
    int source_y_first;
    int source_y_last;
    int viewport_center_x;
    bool frame_resolves_center_column;
    bool uses_f0100_wallset_bitmap;
    bool uses_g0698_wall_d3lcr;
    bool uses_g0163_m600_frame;
    bool uses_c10_transparency;
    bool frame_clip_preserves_c112_byte_width;
    bool wall_case_returns;
    bool calls_f0107_front_alcove_probe;
    int f0107_front_wall_ornament_ordinal;
    int f0107_view_wall_index;
    int f0107_alcove_cell_order;
    bool door_front_draws_d3c_wall_pixels;
    bool stairs_front_draws_d3c_wall_pixels;
    bool stairs_side_draws_d3c_wall_pixels;
    bool pit_draws_d3c_wall_pixels;
    const char *wall_bitmap_symbol;
    const char *frame_symbol;
    const char *source_lock_evidence;
} DM1_V1_D3CWallSpecPc34;

typedef struct {
    DM1_V1_D3CWallSpecPc34 spec;
    bool element_is_wall;
    bool draws_d3c_wall_pixels;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    bool calls_f0107;
    bool f0107_alcove_result;
    bool f0107_alcove_return_path;
    bool returns_after_wall_blit;
    int f0107_ornament_ordinal_arg;
    int f0107_view_wall_arg;
    int alcove_cell_order;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
    const char *source_lock_evidence;
} DM1_V1_D3CWallPixelResultPc34;

#define DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 128
#define DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34 51
#define DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3C_WALL_C112_BYTE_WIDTH_VIEWPORT_PC34 112

const DM1_V1_D3CWallSpecPc34 *dm1_v1_viewport_d3c_wall_spec_pc34(void);

bool dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
    const DM1_V1_D3CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3CWallPixelResultPc34 *out);

uint8_t dm1_v1_viewport_d3c_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d3c_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H */
