#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H

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
} DM1_V1_D1L2WallPixelInputPc34;

typedef struct {
    bool contract_only;
    bool real_asset_bitmap_parity;
    int view_square_index;
    int wall_index_pc34;
    int wall_zone_pc34;
    int frame_index;
    int field_aspect_index;
    int frame_bitmap_index;
    int transparent_color;
    int field_mask;
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
    int byte_width;
    int height;
    bool uses_f0100_frame_blit;
    bool uses_f0104_pc34_zone_blit;
    bool uses_f0105_flipped_blit;
    bool wall_case_returns;
    bool calls_f0107_side_ornament_probe;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    const char *source_lines;
    const char *non_overlap_note;
} DM1_V1_D1L2WallSpecPc34;

typedef struct {
    DM1_V1_D1L2WallSpecPc34 spec;
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
} DM1_V1_D1L2WallPixelResultPc34;

#define DM1_V1_D1L2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1L2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 64
#define DM1_V1_D1L2_WALL_SOURCE_HEIGHT_PC34 111
#define DM1_V1_D1L2_WALL_C10_COLOR_FLESH_PC34 10

const DM1_V1_D1L2WallSpecPc34 *dm1_v1_viewport_d1l2_wall_spec_pc34(void);

bool dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
    const DM1_V1_D1L2WallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1L2WallPixelResultPc34 *out);

uint8_t dm1_v1_viewport_d1l2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d1l2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1L2_WALL_PC34_COMPAT_H */
