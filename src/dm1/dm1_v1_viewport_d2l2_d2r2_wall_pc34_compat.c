#include "dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D2L2_PC34 = 9,     /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    DM1_V1_VIEW_SQUARE_D2R2_PC34 = 10,    /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2 */
    DM1_V1_WALL_D2R2_PC34 = 5,            /* ReDMCSB DEFS.H:3428 C05_WALL_D2R2 */
    DM1_V1_WALL_D2L2_PC34 = 6,            /* ReDMCSB DEFS.H:3429 C06_WALL_D2L2 */
    DM1_V1_ZONE_WALL_D2L2_PC34 = 707,     /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    DM1_V1_ZONE_WALL_D2R2_PC34 = 708      /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
};

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:6837-6865 F0678_DrawD2L2 routes WALL through C06/C707,
 * TELEPORTER through C707, and returns without F0108/F0111/F0115.
 * DUNVIEW.C:6868-6896 F0679_DrawD2R2 mirrors that route through C05/C708.
 * DUNVIEW.C:8503-8508 F0128 places the pair at relative offsets 2,-2
 * then 2,2, so D2L2 draws before D2R2 on the central wall row.
 * DUNVIEW.C:2442-2443 builds the flipped wall-set pair C05<->C06.
 * DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 share the C10
 * transparent blit contract used by the D2L2/D2R2 wall zones.
 * PANEL.C:571 and 578-580 show the panel-side wall/fakewall flag checks
 * using the same C00_ELEMENT_WALL square classification.
 */
static const DM1_V1_D2L2D2R2WallSpecPc34 s_specs[2] = {
    {
        DM1_V1_D2L2_D2R2_WALL_SIDE_D2L2_PC34,
        true,
        false,
        0,
        DM1_V1_VIEW_SQUARE_D2L2_PC34,
        DM1_V1_WALL_D2L2_PC34,
        DM1_V1_WALL_D2R2_PC34,
        DM1_V1_ZONE_WALL_D2L2_PC34,
        0,
        37,
        20,
        90,
        DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L2_D2R2_WALL_SOURCE_HEIGHT_PC34,
        30,
        35,
        0,
        70,
        0,
        5,
        20,
        90,
        6,
        71,
        DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        "C09_VIEW_SQUARE_D2L2",
        "C06_WALL_D2L2",
        "C05_WALL_D2R2",
        "C707_ZONE_WALL_D2L2",
        "DUNVIEW.C:6837-6865 F0678_DrawD2L2; DUNVIEW.C:8503-8504 relative 2,-2"
    },
    {
        DM1_V1_D2L2_D2R2_WALL_SIDE_D2R2_PC34,
        true,
        false,
        1,
        DM1_V1_VIEW_SQUARE_D2R2_PC34,
        DM1_V1_WALL_D2R2_PC34,
        DM1_V1_WALL_D2L2_PC34,
        DM1_V1_ZONE_WALL_D2R2_PC34,
        186,
        223,
        20,
        90,
        DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L2_D2R2_WALL_SOURCE_HEIGHT_PC34,
        0,
        35,
        0,
        70,
        186,
        221,
        20,
        90,
        36,
        71,
        DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        "C10_VIEW_SQUARE_D2R2",
        "C05_WALL_D2R2",
        "C06_WALL_D2L2",
        "C708_ZONE_WALL_D2R2",
        "DUNVIEW.C:6868-6896 F0679_DrawD2R2; DUNVIEW.C:8507-8508 relative 2,2"
    }
};

const DM1_V1_D2L2D2R2WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_wall_spec_pc34(DM1_V1_D2L2D2R2WallSidePc34 side)
{
    if (side == DM1_V1_D2L2_D2R2_WALL_SIDE_D2L2_PC34 ||
        side == DM1_V1_D2L2_D2R2_WALL_SIDE_D2R2_PC34) {
        return &s_specs[(int)side];
    }
    return NULL;
}

static bool dm1_v1_d2l2_d2r2_wall_apply_pixel(
    const DM1_V1_D2L2D2R2WallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2L2D2R2WallPixelPc34 *out)
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
        ? (DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)row * (size_t)DM1_V1_D2L2_D2R2_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;

    if (out->selected_source_x < 0 ||
        out->selected_source_x >= DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 ||
        out->source_offset >= source_len ||
        out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d2l2_d2r2_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

static bool add_check(
    DM1_V1_D2L2D2R2WallRunPc34 *out,
    const DM1_V1_D2L2D2R2WallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    if (out->check_count >= DM1_V1_D2L2_D2R2_WALL_CHECK_CAPACITY_PC34) {
        return false;
    }
    return dm1_v1_d2l2_d2r2_wall_apply_pixel(
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

bool dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(
    DM1_V1_D2L2D2R2WallRunPc34 *out)
{
    uint8_t d2l2_source[DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 *
                        DM1_V1_D2L2_D2R2_WALL_SOURCE_HEIGHT_PC34];
    uint8_t d2r2_source[DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 *
                        DM1_V1_D2L2_D2R2_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D2L2_D2R2_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L2_D2R2_WALL_VIEWPORT_HEIGHT_PC34];
    bool ok = true;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    memset(d2l2_source, DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d2l2_source));
    memset(d2r2_source, DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d2r2_source));
    memset(viewport, 0xee, sizeof(viewport));

    out->d2l2 = &s_specs[DM1_V1_D2L2_D2R2_WALL_SIDE_D2L2_PC34];
    out->d2r2 = &s_specs[DM1_V1_D2L2_D2R2_WALL_SIDE_D2R2_PC34];
    out->c10_palette_index = DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34;
    out->opaque_left_palette_index = 0x42;
    out->opaque_right_palette_index = 0x52;
    out->draw_order_left_before_right =
        out->d2l2->draw_order_index < out->d2r2->draw_order_index;
    out->mirrored_route_pair =
        out->d2l2->native_wall_index_pc34 == out->d2r2->flipped_wall_index_pc34 &&
        out->d2r2->native_wall_index_pc34 == out->d2l2->flipped_wall_index_pc34;
    out->same_wall_zone_family =
        out->d2l2->wall_zone_pc34 + 1 == out->d2r2->wall_zone_pc34;
    out->same_c10_transparency =
        out->d2l2->transparent_color == out->d2r2->transparent_color;
    out->same_height_and_row =
        out->d2l2->visible_height == out->d2r2->visible_height &&
        out->d2l2->viewport_y_first == out->d2r2->viewport_y_first &&
        out->d2l2->viewport_y_last == out->d2r2->viewport_y_last;
    out->excludes_f0108_f0111_f0115 =
        !out->d2l2->calls_f0108_floor_ornament &&
        !out->d2l2->calls_f0111_door &&
        !out->d2l2->calls_f0115_thing_pass &&
        !out->d2r2->calls_f0108_floor_ornament &&
        !out->d2r2->calls_f0111_door &&
        !out->d2r2->calls_f0115_thing_pass;
    out->source_evidence =
        dm1_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34();

    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 30] = 10;
    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 31] = 0x42;
    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 35] = 0x7e;
    d2l2_source[70 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 35] = 0x55;
    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 0] = 10;
    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 1] = 0x52;
    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 35] = 0x5e;
    d2r2_source[70 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 35] = 0x56;

    ok = ok && add_check(out, out->d2l2, false, 20, 0,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, false, 20, 1,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, false, 20, 5,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, false, 20, 6,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, false, 90, 5,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));

    ok = ok && add_check(out, out->d2r2, false, 20, 186,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, false, 20, 187,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, false, 20, 221,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, false, 20, 222,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, false, 90, 221,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));

    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 5] = 10;
    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 4] = 0x63;
    d2r2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6e;
    ok = ok && add_check(out, out->d2l2, true, 20, 0,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, true, 20, 1,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2l2, true, 20, 5,
                         d2r2_source, sizeof(d2r2_source), viewport, sizeof(viewport));

    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 35] = 10;
    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 34] = 0x64;
    d2l2_source[0 * DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6d;
    ok = ok && add_check(out, out->d2r2, true, 20, 186,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, true, 20, 187,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d2r2, true, 20, 221,
                         d2l2_source, sizeof(d2l2_source), viewport, sizeof(viewport));

    out->ok = ok;
    return ok;
}

uint8_t dm1_v1_viewport_d2l2_d2r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "DUNVIEW.C:6837-6865 F0678_DrawD2L2 handles only WALL and "
        "TELEPORTER; lines 6851-6858 draw C05 flipped or C06 native into "
        "C707_ZONE_WALL_D2L2, line 6862 returns, and lines 6863-6865 "
        "draw the teleporter field into the same C707 zone. "
        "DUNVIEW.C:6868-6896 F0679_DrawD2R2 mirrors that route; lines "
        "6882-6889 draw C06 flipped or C05 native into C708_ZONE_WALL_D2R2, "
        "line 6893 returns, and lines 6894-6896 draw the teleporter field "
        "into the same C708 zone. DUNVIEW.C:8503-8508 F0128 places D2L2 "
        "at relative movement 2,-2 before D2R2 at 2,2. DUNVIEW.C:2442-2443 "
        "builds the C05_WALL_D2R2 / C06_WALL_D2L2 flipped wall-set pair. "
        "DUNVIEW.C:3113-3129 F0104 and DUNVIEW.C:3185-3204 F0105 use the "
        "C10_COLOR_FLESH transparent bitmap route; DUNVIEW.C:3048-3058 "
        "F0100 is the sibling frame blit contract locked by adjacent gates. "
        "DEFS.H:2605 C09_VIEW_SQUARE_D2L2=9; DEFS.H:2606 "
        "C10_VIEW_SQUARE_D2R2=10; DEFS.H:3428 C05_WALL_D2R2=5; "
        "DEFS.H:3429 C06_WALL_D2L2=6; DEFS.H:4047 "
        "C707_ZONE_WALL_D2L2=707; DEFS.H:4048 C708_ZONE_WALL_D2R2=708. "
        "COORD.C:2390-2409 F0635 clips MEDIA720 source and viewport zones; "
        "COORD.C:2542-2569 F0640/F0641 load C696_GRAPHIC_LAYOUT. "
        "PANEL.C:571 and PANEL.C:578-580 confirm panel-side square handling "
        "uses the same wall/fakewall flag bits. The requested F0676:6271-6273 "
        "anchor is D3L2 in this source snapshot; the D2L2/D2R2 anchors are "
        "F0678/F0679 and F0128:8503-8508. This gate excludes F0108 floor "
        "ornaments, F0111 doors, and F0115 thing passes for the wall case.";
}
