#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
 * lines 6642-6720 for the D3C wall route; F0100 lines 3048-3058 for
 * the transparent wall-set blit; F0101 lines 3065-3078 for the center
 * no-transparency wall-set route; G0163_aauc_Graphic558_Frame_Walls
 * line 583 for M600_VIEW_SQUARE_D3C frame ordinal 0 and row clipping;
 * G0698_puc_Bitmap_WallSet_Wall_D3LCR for the native source bitmap.
 * CSB-lineage anchor: Viewport.cpp lines 1903-1915, cross-checked with
 * the D3C/F3 table and RF3 dispatch in the same lineage source.
 */

#define CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 224
#define CSB_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34 136
#define CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34 64
#define CSB_V1_D3C_WALL_SOURCE_HEIGHT_PC34 51
#define CSB_V1_D3C_WALL_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D3C_WALL_NO_TRANSPARENCY_PC34 -1

typedef enum {
    CSB_V1_D3C_WALL_ELEMENT_WALL_PC34 = 0,
    CSB_V1_D3C_WALL_ELEMENT_CORRIDOR_PC34 = 1,
    CSB_V1_D3C_WALL_ELEMENT_PIT_PC34 = 2,
    CSB_V1_D3C_WALL_ELEMENT_TELEPORTER_PC34 = 5,
    CSB_V1_D3C_WALL_ELEMENT_DOOR_FRONT_PC34 = 17,
    CSB_V1_D3C_WALL_ELEMENT_STAIRS_SIDE_PC34 = 18,
    CSB_V1_D3C_WALL_ELEMENT_STAIRS_FRONT_PC34 = 19
} CSB_V1_D3CWallElementPc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int source_x;
    int source_y;
} CSB_V1_D3CWallFramePc34;

typedef struct {
    bool contract_only;
    bool real_asset_pixel_parity;
    int view_square_m600;
    int frame_ordinal_m600;
    int view_depth;
    int view_lane;
    CSB_V1_D3CWallFramePc34 frame;
    int effective_source_x1;
    int effective_source_x2;
    int effective_viewport_x1;
    int effective_viewport_x2;
    int effective_visible_width;
    int effective_visible_height;
    int viewport_byte_width;
    int transparent_color;
    int no_transparency_color;
    bool wall_branch_only;
    bool uses_f0118_d3c_wall_route;
    bool uses_g0698_wall_d3lcr;
    bool uses_g0163_m600_frame;
    bool uses_f0101_no_transparency;
    bool preserves_f0100_c10_reference;
    bool rejects_f0121_d2c_path;
    bool calls_f0107_front_alcove_probe;
    bool wall_case_returns_without_alcove;
    bool door_front_draws_d3c_wall_pixels;
    bool stairs_front_draws_d3c_wall_pixels;
    bool stairs_side_draws_d3c_wall_pixels;
    bool pit_draws_d3c_wall_pixels;
    const char *redmcsb_f0118_anchor;
    const char *redmcsb_f0100_anchor;
    const char *redmcsb_f0101_anchor;
    const char *redmcsb_g0163_anchor;
    const char *redmcsb_g0698_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_D3CWallSpecPc34;

typedef struct {
    CSB_V1_D3CWallElementPc34 element;
    int row;
    int viewport_x;
} CSB_V1_D3CWallPixelInputPc34;

typedef struct {
    CSB_V1_D3CWallSpecPc34 spec;
    bool element_is_wall;
    bool draws_d3c_wall_pixels;
    bool in_clip;
    bool writes_pixel;
    bool no_write_metadata;
    bool f0101_no_transparency_write;
    bool f0100_transparent_reference_skip;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
    const char *source_evidence;
} CSB_V1_D3CWallPixelResultPc34;

const CSB_V1_D3CWallSpecPc34 *
csb_v1_viewport_d3c_wall_spec_pc34(void);

bool csb_v1_viewport_d3c_wall_apply_pixel_pc34(
    const CSB_V1_D3CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    CSB_V1_D3CWallPixelResultPc34 *out);

uint8_t csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

uint8_t csb_v1_viewport_d3c_wall_blend_f0101_no_transparency_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *csb_v1_viewport_d3c_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D3C_WALL_PC34_COMPAT_H */
