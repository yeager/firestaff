#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int row;
    int viewport_x;
    uint8_t transparent_color;
} DM1_V1_D2CWallPixelInputPc34;

typedef struct {
    bool contract_only;
    bool real_asset_bitmap_parity;
    int view_square_index;
    int wall_index_pc34;
    int wall_zone_pc34;
    int frame_index;
    int frame_table_byte_width;
    int byte_width;
    int height;
    int transparent_color;
    int source_x_first;
    int source_x_last;
    int source_y_first;
    int source_y_last;
    int viewport_x_first;
    int viewport_x_last;
    int viewport_y_first;
    int viewport_y_last;
    int source_width;
    int source_height;
    bool uses_f0100_c10_transparent_blit;
    bool uses_f0101_pc34_opaque_center_path;
    bool uses_f0104_pc34_native_route;
    bool preserves_c10_transparency;
    bool wall_case_returns;
    bool calls_f0107_front_alcove_probe;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door;
    bool calls_f0113_center_field;
    bool calls_f0115_thing_pass;
    const char *view_square_symbol;
    const char *source_lines;
    const char *non_overlap_note;
} DM1_V1_D2CWallSpecPc34;

typedef struct {
    DM1_V1_D2CWallSpecPc34 spec;
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
} DM1_V1_D2CWallPixelResultPc34;

#define DM1_V1_D2C_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2C_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 72
#define DM1_V1_D2C_WALL_SOURCE_HEIGHT_PC34 71
#define DM1_V1_D2C_WALL_C10_COLOR_FLESH_PC34 10

const DM1_V1_D2CWallSpecPc34 *dm1_v1_viewport_d2c_wall_spec_pc34(void);

bool dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
    const DM1_V1_D2CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2CWallPixelResultPc34 *out);

uint8_t dm1_v1_viewport_d2c_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d2c_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2C_WALL_PC34_COMPAT_H */
