#include "dm1_v1_viewport_d0l_d0r_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D0L_PC34 = 1,  /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L */
    DM1_V1_VIEW_SQUARE_D0R_PC34 = 2,  /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R */
    DM1_V1_WALL_D0R_PC34 = 0,         /* ReDMCSB DEFS.H:3423 C00_WALL_D0R */
    DM1_V1_WALL_D0L_PC34 = 1,         /* ReDMCSB DEFS.H:3424 C01_WALL_D0L */
    DM1_V1_ZONE_WALL_D0L_PC34 = 716,  /* ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L */
    DM1_V1_ZONE_WALL_D0R_PC34 = 717,  /* ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R */
    DM1_V1_ZONE_WALL_FIRST_PC34 = 702, /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    DM1_V1_ZONE_WALL_LAST_PC34 = 717
};

typedef struct {
    DM1_V1_D0LD0RWallRoutePc34 route;
    int view_square_index;
    int selected_wall_bitmap_index;
    int opposite_wall_bitmap_index;
    int pc34_wall_zone_index;
    int source_x_first;
    int source_x_last;
    int viewport_x_first;
    int viewport_x_last;
    bool uses_f0104_native_blit;
    bool uses_f0105_parity_scratch_flip;
} DM1_V1_D0LD0RWallRouteSpecPc34;

static const DM1_V1_D0LD0RWallRouteSpecPc34 s_routes[] = {
    {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        DM1_V1_VIEW_SQUARE_D0L_PC34,
        DM1_V1_WALL_D0L_PC34,
        DM1_V1_WALL_D0R_PC34,
        DM1_V1_ZONE_WALL_D0L_PC34,
        16,
        63,
        0,
        47,
        true,
        false
    },
    {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0R_PARITY_PC34,
        DM1_V1_VIEW_SQUARE_D0R_PC34,
        DM1_V1_WALL_D0L_PC34,
        DM1_V1_WALL_D0R_PC34,
        DM1_V1_ZONE_WALL_D0R_PC34,
        0,
        47,
        16,
        63,
        false,
        true
    }
};

static const DM1_V1_D0LD0RWallRouteSpecPc34 *route_spec(
    DM1_V1_D0LD0RWallRoutePc34 route)
{
    size_t i;
    for (i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (s_routes[i].route == route) return &s_routes[i];
    }
    return NULL;
}

bool M11_GameView_D0LD0RWallResolvePc34(
    const DM1_V1_D0LD0RWallInputPc34 *input,
    DM1_V1_D0LD0RWallSpecPc34 *out)
{
    const DM1_V1_D0LD0RWallRouteSpecPc34 *spec;
    uint8_t transparent_color;

    if (!input || !out) return false;
    spec = route_spec(input->route);
    if (!spec || input->row < 0 ||
        input->row >= DM1_V1_D0L_D0R_WALL_VIEWPORT_HEIGHT_PC34) {
        return false;
    }

    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34;
    }

    memset(out, 0, sizeof(*out));
    out->route = spec->route;
    out->view_square_index = spec->view_square_index;
    out->selected_wall_bitmap_index = spec->selected_wall_bitmap_index;
    out->opposite_wall_bitmap_index = spec->opposite_wall_bitmap_index;
    out->pc34_wall_zone_index = spec->pc34_wall_zone_index;
    out->pc34_wall_zone_family_first = DM1_V1_ZONE_WALL_FIRST_PC34;
    out->pc34_wall_zone_family_last = DM1_V1_ZONE_WALL_LAST_PC34;
    out->source_x_first = spec->source_x_first;
    out->source_x_last = spec->source_x_last;
    out->viewport_x_first = spec->viewport_x_first;
    out->viewport_x_last = spec->viewport_x_last;
    out->source_width = DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34;
    out->source_height = DM1_V1_D0L_D0R_WALL_SOURCE_HEIGHT_PC34;
    out->uses_f0104_native_blit = spec->uses_f0104_native_blit;
    out->uses_f0105_parity_scratch_flip = spec->uses_f0105_parity_scratch_flip;
    out->uses_c10_transparency = true;
    out->wall_case_returns = true;
    out->calls_f0111_door = false;
    out->calls_f0115_thing_pass = false;
    out->calls_f0108_floor_ornament = false;
    out->transparent_color = transparent_color;
    out->source_lines = M11_GameView_D0LD0RWallSourceLockPc34();
    out->contract =
        "Source-locked contract gate only: D0L/D0R WALL side-zone route, "
        "source span, C10 transparency, and parity scratch flip; no full "
        "real-asset wall bitmap parity claim.";
    return true;
}

bool M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
    const DM1_V1_D0LD0RWallSpecPc34 *spec,
    int viewport_x,
    int *source_x,
    int *scratch_x)
{
    int rel_x;
    int mapped_x;

    if (!spec || !source_x || !scratch_x) return false;
    if (viewport_x < spec->viewport_x_first || viewport_x > spec->viewport_x_last) {
        return false;
    }

    rel_x = viewport_x - spec->viewport_x_first;
    if (spec->uses_f0105_parity_scratch_flip) {
        mapped_x = spec->source_x_last - rel_x;
        *scratch_x = spec->source_x_first + rel_x;
    } else {
        mapped_x = spec->source_x_first + rel_x;
        *scratch_x = mapped_x;
    }
    if (mapped_x < 0 || mapped_x >= spec->source_width) return false;

    *source_x = mapped_x;
    return true;
}

bool M11_GameView_D0LD0RWallApplyPixelSlicePc34(
    const DM1_V1_D0LD0RWallInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    int viewport_x,
    DM1_V1_D0LD0RWallPixelPc34 *out)
{
    DM1_V1_D0LD0RWallSpecPc34 spec;
    size_t source_offset;
    size_t viewport_offset;
    int source_x;
    int scratch_x;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!M11_GameView_D0LD0RWallResolvePc34(input, &spec)) return false;
    out->spec = spec;
    out->viewport_x = viewport_x;
    if (!source || !viewport) return false;
    if (!M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
            &spec, viewport_x, &source_x, &scratch_x)) {
        return true;
    }

    source_offset = (size_t)input->row * (size_t)spec.source_width + (size_t)source_x;
    viewport_offset = (size_t)input->row *
                      (size_t)DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 +
                      (size_t)viewport_x;
    if (source_offset >= source_len || viewport_offset >= viewport_len) return false;

    out->visible = 1;
    out->source_x = source_x;
    out->scratch_x = scratch_x;
    out->pixel_before = viewport[viewport_offset];
    out->source_pixel = source[source_offset];
    viewport[viewport_offset] = M11_GameView_D0LD0RWallBlendPixelPc34(
        viewport[viewport_offset], source[source_offset], spec.transparent_color);
    out->pixel_after = viewport[viewport_offset];
    return true;
}

uint8_t M11_GameView_D0LD0RWallBlendPixelPc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *M11_GameView_D0LD0RWallSourceLockPc34(void)
{
    return
        "Source-locked contract gate only; ReDMCSB DUNVIEW.C F0122:7400-7600 "
        "requested side-wall switch anchor, with this snapshot's D0 WALL case "
        "at F0125/F0126 DUNVIEW.C:8007-8038 and 8117-8144. "
        "D0L native route: DUNVIEW.C:8033 F0104(G2107_WallSet[C01_WALL_D0L], "
        "C716_ZONE_WALL_D0L) after the wall case at 8007 and return at 8038. "
        "D0R parity route: DUNVIEW.C:8127 F0105(G2107_WallSet[C01_WALL_D0L], "
        "C717_ZONE_WALL_D0R) and 8139 F0104(G2107_WallSet[C00_WALL_D0R], "
        "C717_ZONE_WALL_D0R), returning at 8144. "
        "DUNVIEW.C:3185-3204 F0105 parity scratch flip copies/flips through "
        "G0074_puc_Bitmap_Temporary and blits with C10_COLOR_FLESH. "
        "DUNVIEW.C:3113-3129 F0104 native blit and DUNVIEW.C:3048-3058 F0100 "
        "share the C10 transparent wall blit contract. "
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:3423-3424 C00_WALL_D0R/"
        "C01_WALL_D0L; DEFS.H:4041 C701 viewport floor-area anchor and "
        "DEFS.H:4042-4057 C702..C717 wall-zone family, including "
        "C716_ZONE_WALL_D0L/C717_ZONE_WALL_D0R. "
        "No F0111_DUNGEONVIEW_DrawDoor, no F0115 thing pass, and no "
        "F0108 floor-ornament call are part of the D0L/D0R WALL return case.";
}
