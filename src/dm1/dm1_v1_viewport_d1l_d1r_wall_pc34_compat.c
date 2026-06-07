#include "dm1_v1_viewport_d1l_d1r_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D1L_PC34 = 4,      /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    DM1_V1_VIEW_SQUARE_D1R_PC34 = 5,      /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R */
    DM1_V1_WALL_D1R_PC34 = 2,             /* ReDMCSB DEFS.H:3425 C02_WALL_D1R */
    DM1_V1_WALL_D1L_PC34 = 3,             /* ReDMCSB DEFS.H:3426 C03_WALL_D1L */
    DM1_V1_ZONE_WALL_D1L_PC34 = 713,      /* ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L */
    DM1_V1_ZONE_WALL_D1R_PC34 = 714,      /* ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R */
    DM1_V1_ZONE_WALL_FIRST_PC34 = 702,    /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    DM1_V1_ZONE_WALL_LAST_PC34 = 717,
    DM1_V1_FIELD_ASPECT_D1L_PC34 = 7,     /* ReDMCSB DUNVIEW.C:727 G0188 D1L row */
    DM1_V1_FIELD_ASPECT_D1R_PC34 = 8      /* ReDMCSB DUNVIEW.C:728 G0188 D1R row */
};

typedef struct {
    DM1_V1_D1LD1RWallRoutePc34 route;
    int lateral;
    int view_square_index;
    int selected_wall_bitmap_index;
    int parity_partner_wall_bitmap_index;
    int wall_zone_index;
    int frame_index;
    int field_aspect_index;
    int field_mask;
    int frame_viewport_x_first;
    int frame_viewport_x_last;
    int frame_source_x;
    bool uses_f0104_native_blit;
    bool uses_f0105_parity_scratch_flip;
} DM1_V1_D1LD1RWallRouteSpecPc34;

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C:581-591 G0163 wall frame rows: D1L viewport 0..63 from source X=192,
 *   D1R viewport 160..223 from source X=0.
 * - DUNVIEW.C:7436-7460 F0122_DUNGEONVIEW_DrawSquareD1L wall case selects
 *   C03_WALL_D1L / C713_ZONE_WALL_D1L and returns.
 * - DUNVIEW.C:7604-7628 F0123_DUNGEONVIEW_DrawSquareD1R wall case has the
 *   parity route F0105(C03_WALL_D1L, C714_ZONE_WALL_D1R) and returns.
 * - DUNVIEW.C:3048-3058 F0100 and 3185-3204 F0105 keep C10 transparent blits.
 * - DRAWVIEW.C:709-722 F0097 presents the prepared viewport after draw work.
 */
static const DM1_V1_D1LD1RWallRouteSpecPc34 s_routes[] = {
    {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1L_NATIVE_PC34,
        -1,
        DM1_V1_VIEW_SQUARE_D1L_PC34,
        DM1_V1_WALL_D1L_PC34,
        DM1_V1_WALL_D1R_PC34,
        DM1_V1_ZONE_WALL_D1L_PC34,
        DM1_V1_VIEW_SQUARE_D1L_PC34,
        DM1_V1_FIELD_ASPECT_D1L_PC34,
        0x82,
        0,
        63,
        192,
        true,
        false
    },
    {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1R_PARITY_PC34,
        1,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        DM1_V1_WALL_D1L_PC34,
        DM1_V1_WALL_D1R_PC34,
        DM1_V1_ZONE_WALL_D1R_PC34,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        DM1_V1_FIELD_ASPECT_D1R_PC34,
        0x02,
        160,
        223,
        192,
        false,
        true
    }
};

static const DM1_V1_D1LD1RWallRouteSpecPc34 *route_spec(
    DM1_V1_D1LD1RWallRoutePc34 route)
{
    size_t i;
    for (i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (s_routes[i].route == route) return &s_routes[i];
    }
    return NULL;
}

bool M11_GameView_D1LD1RWallResolvePc34(
    const DM1_V1_D1LD1RWallInputPc34 *input,
    DM1_V1_D1LD1RWallSpecPc34 *out)
{
    const DM1_V1_D1LD1RWallRouteSpecPc34 *spec;
    uint8_t transparent_color;

    if (!input || !out) return false;
    spec = route_spec(input->route);
    if (!spec) return false;

    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34;
    }

    memset(out, 0, sizeof(*out));
    out->route = spec->route;
    out->contract_only = true;
    out->real_asset_bitmap_parity = false;
    out->depth = 1;
    out->lateral = spec->lateral;
    out->view_square_index = spec->view_square_index;
    out->selected_wall_bitmap_index = spec->selected_wall_bitmap_index;
    out->parity_partner_wall_bitmap_index = spec->parity_partner_wall_bitmap_index;
    out->wall_zone_index = spec->wall_zone_index;
    out->wall_zone_family_first = DM1_V1_ZONE_WALL_FIRST_PC34;
    out->wall_zone_family_last = DM1_V1_ZONE_WALL_LAST_PC34;
    out->frame_index = spec->frame_index;
    out->field_aspect_index = spec->field_aspect_index;
    out->field_mask = spec->field_mask;
    out->frame_viewport_x_first = spec->frame_viewport_x_first;
    out->frame_viewport_x_last = spec->frame_viewport_x_last;
    out->frame_viewport_y_first = 9;
    out->frame_viewport_y_last = 119;
    out->frame_source_x = spec->frame_source_x;
    out->frame_source_y = 0;
    out->frame_byte_width = 128;
    out->frame_height = 111;
    out->source_x_first = 192;
    out->source_x_last = 255;
    out->source_y_first = 0;
    out->source_y_last = 110;
    out->source_width = DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34;
    out->source_height = DM1_V1_D1L_D1R_WALL_SOURCE_HEIGHT_PC34;
    out->uses_f0100_frame_blit = true;
    out->uses_f0104_native_blit = spec->uses_f0104_native_blit;
    out->uses_f0105_parity_scratch_flip = spec->uses_f0105_parity_scratch_flip;
    out->uses_c10_transparency = true;
    out->wall_case_returns = true;
    out->calls_f0107_side_ornament_probe = true;
    out->calls_f0108_floor_ornament = false;
    out->calls_f0111_door = false;
    out->calls_f0115_thing_pass = false;
    out->transparent_color = transparent_color;
    out->source_lines = M11_GameView_D1LD1RWallSourceLockPc34();
    out->contract =
        "Source-locked contract gate only: D1L/D1R depth-1 near side-wall "
        "route, C03/C02 wall selection, D1R parity flip, C713/C714 zones, "
        "C10 transparency, and frame/clip metadata; no full real-asset "
        "wall bitmap parity claim.";
    return true;
}

bool M11_GameView_D1LD1RWallMapViewportToSourcePc34(
    const DM1_V1_D1LD1RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    int *source_x,
    int *source_y,
    int *scratch_x)
{
    int rel_x;
    int rel_y;

    if (!spec || !source_x || !source_y || !scratch_x) return false;
    if (row < spec->frame_viewport_y_first || row > spec->frame_viewport_y_last ||
        viewport_x < spec->frame_viewport_x_first ||
        viewport_x > spec->frame_viewport_x_last) {
        return false;
    }

    rel_x = viewport_x - spec->frame_viewport_x_first;
    rel_y = row - spec->frame_viewport_y_first;
    if (spec->uses_f0105_parity_scratch_flip) {
        *source_x = spec->source_x_last - rel_x;
        *scratch_x = spec->source_x_first + rel_x;
    } else {
        *source_x = spec->source_x_first + rel_x;
        *scratch_x = *source_x;
    }
    *source_y = spec->source_y_first + rel_y;

    if (*source_x < 0 || *source_x >= spec->source_width ||
        *source_y < 0 || *source_y >= spec->source_height) {
        return false;
    }
    return true;
}

bool M11_GameView_D1LD1RWallApplyPixelPc34(
    const DM1_V1_D1LD1RWallInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1LD1RWallPixelPc34 *out)
{
    DM1_V1_D1LD1RWallSpecPc34 spec;
    int source_x;
    int source_y;
    int scratch_x;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!M11_GameView_D1LD1RWallResolvePc34(input, &spec)) return false;

    out->spec = spec;
    out->row = input->row;
    out->viewport_x = input->viewport_x;
    if (!M11_GameView_D1LD1RWallMapViewportToSourcePc34(
            &spec, input->row, input->viewport_x, &source_x, &source_y, &scratch_x)) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = source_x;
    out->source_y = source_y;
    out->scratch_x = scratch_x;
    out->source_offset = (size_t)source_y * (size_t)spec.source_width +
        (size_t)source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D1L_D1R_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == spec.transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] = M11_GameView_D1LD1RWallBlendPixelPc34(
        viewport[out->viewport_offset], out->source_pixel, spec.transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t M11_GameView_D1LD1RWallBlendPixelPc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *M11_GameView_D1LD1RWallSourceLockPc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "This ReDMCSB snapshot numbers the requested depth-1 side-wall "
        "routines as F0122/F0123: DUNVIEW.C:8524-8529 dispatches "
        "depth=1,lateral=-1 to F0122_DUNGEONVIEW_DrawSquareD1L and "
        "depth=1,lateral=+1 to F0123_DUNGEONVIEW_DrawSquareD1R. "
        "DUNVIEW.C:7436-7460 F0122 D1L WALL draws "
        "G0700_puc_Bitmap_WallSet_Wall_D1LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M607_VIEW_SQUARE_D1L], selects "
        "G2107_WallSet[C03_WALL_D1L] / C713_ZONE_WALL_D1L, calls F0107, "
        "then returns. DUNVIEW.C:7604-7628 F0123 D1R WALL draws "
        "G0700_puc_Bitmap_WallSet_Wall_D1LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M608_VIEW_SQUARE_D1R], has the "
        "parity route F0105(G2107_WallSet[C03_WALL_D1L], "
        "C714_ZONE_WALL_D1R), the native C02 route at DUNVIEW.C:7622, calls "
        "F0107, then returns. DUNVIEW.C:581-591 gives D1L frame "
        "{0..63, y=9..119, byte_width=128, height=111, source_x=192} and "
        "D1R frame {160..223, y=9..119, byte_width=128, height=111, "
        "source_x=0}. DUNVIEW.C:718-731 G0188 field aspects give D1L mask "
        "0x82 and D1R mask 0x02. DUNVIEW.C:2438-2439 pairs C02_WALL_D1R "
        "and C03_WALL_D1L as horizontal-flip partners. DUNVIEW.C:3048-3058 "
        "F0100_DUNGEONVIEW_DrawWallSetBitmap and DUNVIEW.C:3185-3204 "
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally "
        "blit with C10_COLOR_FLESH. DRAWVIEW.C:709-722 F0097 presents the "
        "prepared viewport. DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2600-2601 "
        "M607_VIEW_SQUARE_D1L/M608_VIEW_SQUARE_D1R; DEFS.H:3425-3426 "
        "C02_WALL_D1R/C03_WALL_D1L; DEFS.H:4053-4054 "
        "C713_ZONE_WALL_D1L/C714_ZONE_WALL_D1R. PANEL.C contains no "
        "symbolic C713/C714 row names in this snapshot; DUNVIEW.C and "
        "DEFS.H provide the source zone constants used by the gate.";
}
