#include "csb/csb_v1_viewport_d1l_d1r_wall_pc34_compat.h"

#include "csb_v1_boot.h"
#include "csb_v1_pc34_wallset_graphics_map.h"

#include <stdlib.h>
#include <string.h>

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D1L = 1,
    CSB_SIDE_D1R = 2,
    CSB_ELEMENT_WALL = 0,
    CSB_ELEMENT_TELEPORTER = 5,
    CSB_VIEW_SQUARE_D1C = 3,
    CSB_VIEW_SQUARE_D1L = 4,
    CSB_VIEW_SQUARE_D1R = 5,
    CSB_VIEW_DEPTH_D1 = 1,
    CSB_VIEW_LATERAL_D1L = -1,
    CSB_VIEW_LATERAL_D1R = 1,
    CSB_WALL_D1R = 2,
    CSB_WALL_D1L = 3,
    CSB_ZONE_WALL_D1C = 712,
    CSB_ZONE_WALL_D1L = 713,
    CSB_ZONE_WALL_D1R = 714,
    CSB_C10_COLOR_FLESH = 10,
    CSB_SCREEN_WIDTH = 320,
    CSB_SCREEN_HEIGHT = 200,
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_D1_SIDE_SOURCE_WIDTH = 60,
    CSB_D1_SIDE_SOURCE_HEIGHT = 111,
    CSB_ZONE_MATH_BASE = 709
};

static const char s_source_evidence[] =
    "PC3.4 source-bound CSB V1 D1L/D1R wall pair. ReDMCSB "
    "DUNVIEW.C:7391-7560 F0122 and DUNVIEW.C:7559-7725 F0123 select "
    "G2107_WallSet[C03_WALL_D1L] and G2107_WallSet[C02_WALL_D1R]; "
    "F0095_LoadWallSet resolves them as GRAPHICS.DAT records "
    "86 + WallSet*40 + C03/C02. F0104/F0105 preserve "
    "C10_COLOR_FLESH in C713/C714. dmweb's GRAPHICS.DAT format reference "
    "and CSBWin's ExpandGraphic lineage require decoding the original "
    "compressed record; no generated bitmap or generic source buffer is "
    "accepted.";

static const CSB_V1_D1LD1RWallSpecPc34 s_specs[] = {
    { CSB_PRESENT, CSB_ABSENT, CSB_ABSENT, CSB_PRESENT, CSB_PRESENT,
      CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      CSB_SIDE_D1L, 122, 0, CSB_VIEW_SQUARE_D1L, CSB_VIEW_DEPTH_D1,
      CSB_VIEW_LATERAL_D1L, CSB_ELEMENT_WALL, CSB_ELEMENT_TELEPORTER,
      CSB_ZONE_WALL_D1L, CSB_ZONE_WALL_D1C, CSB_WALL_D1L, CSB_WALL_D1R,
      7, 0, 63, 9, 119, 128, 111, 192, 0, 64, 111,
      CSB_C10_COLOR_FLESH, CSB_SCREEN_WIDTH, CSB_SCREEN_HEIGHT,
      CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT, CSB_PRESENT, CSB_PRESENT,
      CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      CSB_ABSENT, CSB_ABSENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      "D1L wall body via F0122", "C713_ZONE_WALL_D1L",
      "G2107_WallSet[C03_WALL_D1L]", "ReDMCSB DUNVIEW.C:7391-7560 F0122" },
    { CSB_PRESENT, CSB_ABSENT, CSB_ABSENT, CSB_PRESENT, CSB_PRESENT,
      CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      CSB_SIDE_D1R, 123, 1, CSB_VIEW_SQUARE_D1R, CSB_VIEW_DEPTH_D1,
      CSB_VIEW_LATERAL_D1R, CSB_ELEMENT_WALL, CSB_ELEMENT_TELEPORTER,
      CSB_ZONE_WALL_D1R, CSB_ZONE_WALL_D1C, CSB_WALL_D1R, CSB_WALL_D1L,
      8, 160, 223, 9, 119, 128, 111, 0, 0, 64, 111,
      CSB_C10_COLOR_FLESH, CSB_SCREEN_WIDTH, CSB_SCREEN_HEIGHT,
      CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT, CSB_PRESENT, CSB_PRESENT,
      CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      CSB_ABSENT, CSB_ABSENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
      "D1R wall body via F0123", "C714_ZONE_WALL_D1R",
      "G2107_WallSet[C02_WALL_D1R]", "ReDMCSB DUNVIEW.C:7559-7725 F0123" }
};

size_t csb_v1_viewport_d1l_d1r_wall_spec_count_pc34(void)
{ return sizeof(s_specs) / sizeof(s_specs[0]); }

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(size_t index)
{ return index < csb_v1_viewport_d1l_d1r_wall_spec_count_pc34() ? &s_specs[index] : NULL; }

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(int side)
{ return side == CSB_SIDE_D1L ? &s_specs[0] : side == CSB_SIDE_D1R ? &s_specs[1] : NULL; }

int csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(int square)
{ return square == CSB_VIEW_SQUARE_D1L || square == CSB_VIEW_SQUARE_D1R ? CSB_ZONE_MATH_BASE + square : -1; }

int csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec, int viewport_x,
    int flipped_variant, int *out_source_x)
{
    int local_x;
    if (!spec || !out_source_x) return -1;
    if (viewport_x < spec->frame_x1 || viewport_x > spec->frame_x2) return 1;
    local_x = viewport_x - spec->frame_x1;
    if (local_x < 0 || local_x >= spec->clip_width) return -1;
    *out_source_x = flipped_variant ? spec->clip_width - 1 - local_x : local_x;
    return 0;
}

int csb_v1_viewport_d1l_d1r_wall_load_graphics_dat_material_pc34(
    const char *path, int wall_set, int side, uint32_t graphics_entry_count,
    unsigned char **out_pixels, CSB_V1_D1LD1RWallMaterialPc34 *out_material)
{
    CSB_V1_PC34WallSetSurface surface;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
    unsigned char *pixels = NULL;
    uint32_t index = 0u;
    int width = 0;
    int height = 0;

    /* ReDMCSB PC/I34 DUNVIEW.C F0095_LoadWallSet writes the first 22
     * GRAPHICS.DAT records as G2107 wall surfaces. F0122 uses C03 for D1L
     * and F0123 uses C02 for D1R; F0104/F0105 then consume the native pixels
     * with C10 transparency. dmweb documents this original record stream and
     * CSBWin's ExpandGraphic is the cross-checked decoder lineage. */
    if (out_pixels) *out_pixels = NULL;
    if (out_material) memset(out_material, 0, sizeof(*out_material));
    if (!path || !path[0] || !out_pixels || !out_material || wall_set < 0) return 0;
    if (side == CSB_SIDE_D1L) surface = CSB_V1_PC34_WALLSET_WALL_D1L;
    else if (side == CSB_SIDE_D1R) surface = CSB_V1_PC34_WALLSET_WALL_D1R;
    else return 0;
    if (!csb_v1_pc34_wallset_graphics_entry_index(
            wall_set, surface, graphics_entry_count, &index) ||
        !csb_v1_boot_decode_graphics_dat_asset_pc34(
            path, index, &pixels, &width, &height, &receipt) ||
        !pixels || !receipt.valid || width != CSB_D1_SIDE_SOURCE_WIDTH ||
        height != CSB_D1_SIDE_SOURCE_HEIGHT ||
        !receipt.compressed_record_sha256[0]) {
        free(pixels);
        return 0;
    }
    out_material->valid = 1;
    out_material->wall_set = wall_set;
    out_material->side = side;
    out_material->graphics_entry_index = index;
    out_material->width = width;
    out_material->height = height;
    out_material->transparent_color = CSB_C10_COLOR_FLESH;
    out_material->flip_horizontally = 0;
    out_material->decode_receipt = receipt;
    *out_pixels = pixels;
    return 1;
}

uint16_t csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec, uint16_t first_thing)
{ return spec && spec->f0115_thing_pass_keepout ? first_thing : 0xffffu; }

int csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec, int x, int y, int *out_x, int *out_y)
{
    if (!spec || !out_x || !out_y || !spec->f0172_square_aspect_read) return -1;
    *out_x = x; *out_y = y; return 0;
}

const char *csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34(void)
{ return s_source_evidence; }
