#include "dm1_v1_viewport_d3l_d3r_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D3L_PC34 = 12,     /* ReDMCSB DEFS.H:2608 M601_VIEW_SQUARE_D3L */
    DM1_V1_VIEW_SQUARE_D3R_PC34 = 13,     /* ReDMCSB DEFS.H:2609 M602_VIEW_SQUARE_D3R */
    DM1_V1_WALL_D3R_PC34 = 12,            /* ReDMCSB DEFS.H:3435 C12_WALL_D3R */
    DM1_V1_WALL_D3L_PC34 = 13,            /* ReDMCSB DEFS.H:3436 C13_WALL_D3L */
    DM1_V1_ZONE_WALL_D3L2_PC34 = 702,     /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    DM1_V1_ZONE_WALL_D3R2_PC34 = 703,     /* ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2 */
    DM1_V1_ZONE_WALL_D3L_PC34 = 705,      /* ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L */
    DM1_V1_ZONE_WALL_D3R_PC34 = 706,      /* ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R */
    DM1_V1_ZONE_WALL_D2L2_PC34 = 707,     /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    DM1_V1_ZONE_WALL_D2R2_PC34 = 708,     /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
    DM1_V1_ZONE_WALL_D2L_PC34 = 710,      /* ReDMCSB DEFS.H:4050 C710_ZONE_WALL_D2L */
    DM1_V1_ZONE_WALL_D2R_PC34 = 711       /* ReDMCSB DEFS.H:4051 C711_ZONE_WALL_D2R */
};

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:581-585 defines the D3L/D3R frame metadata. D3L starts at
 * source X=32 and clips to viewport X=0..31; D3R starts at source X=0
 * and clips to viewport X=139..202.
 * DUNVIEW.C:6406-6437 F0116_DUNGEONVIEW_DrawSquareD3L routes WALL through
 * C13_WALL_D3L/C705 or flipped C12_WALL_D3R/C705, then returns unless a
 * front alcove explicitly hands off to F0115.
 * DUNVIEW.C:6545-6573 F0117_DUNGEONVIEW_DrawSquareD3R mirrors the route
 * through C12_WALL_D3R/C706 or flipped C13_WALL_D3L/C706.
 * DUNVIEW.C:2434-2435 builds the C12<->C13 flipped wall-set pair.
 * DUNVIEW.C:3048-3058 F0100 and DUNVIEW.C:3113-3204 F0104/F0105 share the
 * C10 transparent blit contract used by the D3L/D3R wall zones.
 * DUNVIEW.C:8490-8495 draws D3L at relative 3,-1 before D3R at 3,1.
 */
static const DM1_V1_D3LD3RWallSpecPc34 s_specs[2] = {
    {
        DM1_V1_D3L_D3R_WALL_SIDE_D3L_PC34,
        true,
        false,
        0,
        DM1_V1_VIEW_SQUARE_D3L_PC34,
        3,
        -1,
        DM1_V1_WALL_D3L_PC34,
        DM1_V1_WALL_D3R_PC34,
        DM1_V1_ZONE_WALL_D3L_PC34,
        0,
        83,
        25,
        75,
        DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D3L_D3R_WALL_SOURCE_HEIGHT_PC34,
        32,
        0,
        32,
        63,
        0,
        50,
        0,
        31,
        25,
        75,
        32,
        51,
        DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        "M601_VIEW_SQUARE_D3L",
        "C13_WALL_D3L",
        "C12_WALL_D3R",
        "C705_ZONE_WALL_D3L",
        "DUNVIEW.C:6406-6437 F0116_DUNGEONVIEW_DrawSquareD3L; DUNVIEW.C:8490-8491 relative 3,-1"
    },
    {
        DM1_V1_D3L_D3R_WALL_SIDE_D3R_PC34,
        true,
        false,
        1,
        DM1_V1_VIEW_SQUARE_D3R_PC34,
        3,
        1,
        DM1_V1_WALL_D3R_PC34,
        DM1_V1_WALL_D3L_PC34,
        DM1_V1_ZONE_WALL_D3R_PC34,
        139,
        223,
        25,
        75,
        DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D3L_D3R_WALL_SOURCE_HEIGHT_PC34,
        0,
        0,
        0,
        63,
        0,
        50,
        139,
        202,
        25,
        75,
        64,
        51,
        DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        "M602_VIEW_SQUARE_D3R",
        "C12_WALL_D3R",
        "C13_WALL_D3L",
        "C706_ZONE_WALL_D3R",
        "DUNVIEW.C:6545-6573 F0117_DUNGEONVIEW_DrawSquareD3R; DUNVIEW.C:8494-8495 relative 3,1"
    }
};

const DM1_V1_D3LD3RWallSpecPc34 *
dm1_v1_viewport_d3l_d3r_wall_spec_pc34(DM1_V1_D3LD3RWallSidePc34 side)
{
    if (side == DM1_V1_D3L_D3R_WALL_SIDE_D3L_PC34 ||
        side == DM1_V1_D3L_D3R_WALL_SIDE_D3R_PC34) {
        return &s_specs[(int)side];
    }
    return NULL;
}

static bool dm1_v1_d3l_d3r_wall_apply_pixel(
    const DM1_V1_D3LD3RWallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3LD3RWallPixelPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->parity_flip = parity_flip;
    out->row = row;
    out->viewport_x = viewport_x;
    if (!spec) return false;

    if (row < spec->viewport_y_first ||
        row > spec->viewport_y_last ||
        viewport_x < spec->viewport_x_first ||
        viewport_x > spec->viewport_x_last) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = spec->source_x_first + (viewport_x - spec->viewport_x_first);
    out->source_y = spec->source_y_first + (row - spec->viewport_y_first);
    out->selected_source_x = parity_flip
        ? (DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)row * (size_t)DM1_V1_D3L_D3R_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;

    if (out->selected_source_x < 0 ||
        out->selected_source_x >= DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 ||
        out->source_offset >= source_len ||
        out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d3l_d3r_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

static bool add_check(
    DM1_V1_D3LD3RWallRunPc34 *out,
    const DM1_V1_D3LD3RWallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    if (out->check_count >= DM1_V1_D3L_D3R_WALL_CHECK_CAPACITY_PC34) {
        return false;
    }
    return dm1_v1_d3l_d3r_wall_apply_pixel(
        spec,
        parity_flip,
        row,
        viewport_x,
        source,
        source_len,
        viewport,
        viewport_len,
        &out->checks[out->check_count++]);
}

bool dm1_v1_viewport_d3l_d3r_wall_pc34_compat_run(
    DM1_V1_D3LD3RWallRunPc34 *out)
{
    uint8_t d3l_source[DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 *
                       DM1_V1_D3L_D3R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t d3r_source[DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 *
                       DM1_V1_D3L_D3R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3L_D3R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3L_D3R_WALL_VIEWPORT_HEIGHT_PC34];
    bool ok = true;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    memset(d3l_source, DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d3l_source));
    memset(d3r_source, DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d3r_source));
    memset(viewport, 0xee, sizeof(viewport));

    out->d3l = &s_specs[DM1_V1_D3L_D3R_WALL_SIDE_D3L_PC34];
    out->d3r = &s_specs[DM1_V1_D3L_D3R_WALL_SIDE_D3R_PC34];
    out->c10_palette_index = DM1_V1_D3L_D3R_WALL_C10_COLOR_FLESH_PC34;
    out->exact_d3l2_zone_pc34 = DM1_V1_ZONE_WALL_D3L2_PC34;
    out->exact_d3r2_zone_pc34 = DM1_V1_ZONE_WALL_D3R2_PC34;
    out->exact_d2l2_zone_pc34 = DM1_V1_ZONE_WALL_D2L2_PC34;
    out->exact_d2r2_zone_pc34 = DM1_V1_ZONE_WALL_D2R2_PC34;
    out->exact_d2l_zone_pc34 = DM1_V1_ZONE_WALL_D2L_PC34;
    out->exact_d2r_zone_pc34 = DM1_V1_ZONE_WALL_D2R_PC34;
    out->draw_order_left_before_right =
        out->d3l->draw_order_index < out->d3r->draw_order_index;
    out->mirrored_route_pair =
        out->d3l->native_wall_index_pc34 == out->d3r->flipped_wall_index_pc34 &&
        out->d3r->native_wall_index_pc34 == out->d3l->flipped_wall_index_pc34;
    out->d3l_d3r_zone_pair =
        out->d3l->wall_zone_pc34 == DM1_V1_ZONE_WALL_D3L_PC34 &&
        out->d3r->wall_zone_pc34 == DM1_V1_ZONE_WALL_D3R_PC34;
    out->d3l2_d3r2_not_c707_c708 =
        out->exact_d3l2_zone_pc34 != DM1_V1_ZONE_WALL_D2L2_PC34 &&
        out->exact_d3r2_zone_pc34 != DM1_V1_ZONE_WALL_D2R2_PC34;
    out->d2l2_d2r2_are_c707_c708 =
        out->exact_d2l2_zone_pc34 == 707 && out->exact_d2r2_zone_pc34 == 708;
    out->d2l_d2r_are_c710_c711 =
        out->exact_d2l_zone_pc34 == 710 && out->exact_d2r_zone_pc34 == 711;
    out->same_c10_transparency =
        out->d3l->transparent_color == out->d3r->transparent_color;
    out->same_height_and_row =
        out->d3l->visible_height == out->d3r->visible_height &&
        out->d3l->viewport_y_first == out->d3r->viewport_y_first &&
        out->d3l->viewport_y_last == out->d3r->viewport_y_last;
    out->d3r_is_horizontal_mirror_source =
        out->d3l->source_x_first + out->d3l->visible_width - 1 ==
        DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 - 1 &&
        out->d3r->source_x_first == 0;
    out->excludes_f0108_f0111_on_wall =
        !out->d3l->calls_f0108_floor_ornament_on_wall &&
        !out->d3l->calls_f0111_door_on_wall &&
        !out->d3r->calls_f0108_floor_ornament_on_wall &&
        !out->d3r->calls_f0111_door_on_wall;
    out->source_evidence =
        dm1_v1_viewport_d3l_d3r_wall_source_evidence_pc34();

    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 32] = 10;
    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 33] = 0x42;
    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 63] = 0x7e;
    d3l_source[50 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 63] = 0x55;
    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 0] = 10;
    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 1] = 0x52;
    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 63] = 0x5e;
    d3r_source[50 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 63] = 0x56;

    ok = ok && add_check(out, out->d3l, false, 25, 0,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, false, 25, 1,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, false, 25, 31,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, false, 25, 32,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, false, 75, 31,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));

    ok = ok && add_check(out, out->d3r, false, 25, 139,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, false, 25, 140,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, false, 25, 202,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, false, 25, 203,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, false, 75, 202,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));

    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 31] = 10;
    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 30] = 0x63;
    d3r_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6e;
    ok = ok && add_check(out, out->d3l, true, 25, 0,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, true, 25, 1,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l, true, 25, 31,
                         d3r_source, sizeof(d3r_source), viewport, sizeof(viewport));

    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 63] = 10;
    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 62] = 0x64;
    d3l_source[0 * DM1_V1_D3L_D3R_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6d;
    ok = ok && add_check(out, out->d3r, true, 25, 139,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, true, 25, 140,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r, true, 25, 202,
                         d3l_source, sizeof(d3l_source), viewport, sizeof(viewport));

    out->ok = ok;
    return ok;
}

uint8_t dm1_v1_viewport_d3l_d3r_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d3l_d3r_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "DUNVIEW.C:581-585 defines G0163 D3L as "
        "{0,83,25,75,64,51,32,0} and D3R as "
        "{139,223,25,75,64,51,0,0}. "
        "DUNVIEW.C:6406-6437 F0116_DUNGEONVIEW_DrawSquareD3L handles WALL "
        "through F0100 for old media and through C13_WALL_D3L/C705 or "
        "flipped C12_WALL_D3R/C705 on PC34; lines 6432-6437 return unless "
        "a front alcove explicitly routes to F0115. "
        "DUNVIEW.C:6545-6573 F0117_DUNGEONVIEW_DrawSquareD3R mirrors that "
        "route through C12_WALL_D3R/C706 or flipped C13_WALL_D3L/C706; "
        "lines 6568-6573 have the matching alcove/return branch. "
        "DUNVIEW.C:8490-8495 places D3L at relative movement 3,-1 before "
        "D3R at 3,1 after D3L2/D3R2 and before D3C. "
        "DUNVIEW.C:2434-2435 builds the C12_WALL_D3R / C13_WALL_D3L "
        "flipped wall-set pair; the requested C04/C05 names are not the "
        "D3L/D3R wall symbols in this source snapshot. "
        "DUNVIEW.C:3048-3058 F0100 forwards the wall-set frame to C10 "
        "transparent blit, and DUNVIEW.C:3113-3204 F0104/F0105 use the "
        "same C10_COLOR_FLESH transparent route. DRAWVIEW.C:709-723 "
        "F0097 presents the viewport after the dungeon-view bitmap is "
        "prepared. DEFS.H:2608 M601_VIEW_SQUARE_D3L=12; DEFS.H:2609 "
        "M602_VIEW_SQUARE_D3R=13; DEFS.H:3435 C12_WALL_D3R=12; "
        "DEFS.H:3436 C13_WALL_D3L=13; DEFS.H:4042-4043 shows D3L2/D3R2 "
        "are C702/C703, not C707/C708; DEFS.H:4045 C705_ZONE_WALL_D3L=705; "
        "DEFS.H:4046 C706_ZONE_WALL_D3R=706; DEFS.H:4047-4048 C707/C708 "
        "are D2L2/D2R2; DEFS.H:4050-4051 C710/C711 are D2L/D2R. "
        "COORD.C:2390-2409 F0635 clips MEDIA720 source and viewport zones; "
        "IMAGE3.C:866-889 is the source row copy used by adjacent gates. "
        "The wall case does not call F0108 floor ornaments or F0111 doors; "
        "F0115 only follows the explicit front-alcove path.";
}
