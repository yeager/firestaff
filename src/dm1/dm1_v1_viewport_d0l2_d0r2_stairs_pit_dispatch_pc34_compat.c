#include "dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source lock for the D0L2/D0R2 parity alias over the
 * ReDMCSB D3L/D3R side-lane bodies:
 * - DUNVIEW.C F0116:6361-6480 and F0117:6500-6622 dispatch stairs-front
 *   and floor-pit paths, then tail-call F0115 from those same bodies.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 define the transparent
 *   native and flipped bitmap contracts used by those paths.
 * - DUNVIEW.C F0128:8478-8508 reaches F0116/F0117 after D3L2/D3R2 and
 *   before D2L2/D2R2, keeping this gate separate from the older D2L2 work.
 */

static const DM1_V1_D0L2D0R2StairsPitAnchorPc34 s_anchors[] = {
    {
        "d0l2_dispatch", "DUNVIEW.C", "F0116_DUNGEONVIEW_DrawSquareD3L",
        6361, 6480,
        "D0L2 parity alias: stairs-front and floor-pit routes through F0104."
    },
    {
        "d0r2_dispatch", "DUNVIEW.C", "F0117_DUNGEONVIEW_DrawSquareD3R",
        6500, 6622,
        "D0R2 parity alias: mirrored stairs-front and floor-pit routes through F0105."
    },
    {
        "native_blit", "DUNVIEW.C",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        3113, 3156,
        "Native bitmap path preserves C10 transparent pixels."
    },
    {
        "flipped_blit", "DUNVIEW.C",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
        3185, 3247,
        "Mirrored bitmap path flips horizontally and keeps C10 transparency."
    },
    {
        "thing_pass", "DUNVIEW.C",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        4547, 4581,
        "Thing pass reached at F0116 line 6480 and F0117 line 6622."
    },
    {
        "projectile_negative_row_guard", "DUNVIEW.C", "F0115 row guard",
        5668, 5671,
        "Negative G2028 rows skip projectile processing for view squares."
    },
    {
        "main_draw_order", "DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF",
        8478, 8508,
        "D3L2/D3R2 precede F0116/F0117; D2L2/D2R2 are later neighbors."
    },
    {
        "thing_link", "DUNGEON.C", "F0163_DUNGEON_LinkThingToList",
        1769, 1838,
        "Thing-list map-coordinate lineage before square aspect dispatch."
    },
    {
        "thing_unlink", "DUNGEON.C", "F0164_DUNGEON_UnlinkThingFromList",
        1840, 1905,
        "Thing-list removal keeps the same map-coordinate lineage."
    },
    {
        "aspect", "DUNGEON.C", "F0172_DUNGEON_SetSquareAspect",
        2466, 2523,
        "F0116/F0117 read square aspect from direction/map X/map Y."
    },
    {
        "defs_requested", "DEFS.H", "requested D0L2/D0R2 source constants",
        2088, 4207,
        "C10, C02/C09, M604/M605, D3L2/D3R2 view IDs, C0x0021/C0x3421/C0x4312, zones."
    },
    {
        "defs_exact_d3lr", "DEFS.H", "exact F0116/F0117 PC34 constants",
        2441, 4201,
        "C00/C07 slots, M601/M602 view IDs, C802/C804/C815/C817, C852/C854."
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no game-data load. "
    "DUNVIEW.C:F0116_DUNGEONVIEW_DrawSquareD3L:6361-6480 is the D0L2 "
    "parity alias body: lines 6375-6405 route stairs-front through F0104, "
    "lines 6461-6472 route open floor pits through F0104, lines 6475-6480 "
    "set C0x3421 and dispatch F0115 from this body with M601_VIEW_SQUARE_D3L. "
    "DUNVIEW.C:F0117_DUNGEONVIEW_DrawSquareD3R:6500-6622 is the D0R2 "
    "parity alias body: lines 6514-6544 mirror stairs-front through F0105, "
    "lines 6603-6614 mirror open floor pits through F0105, lines 6617-6622 "
    "set C0x4312 and dispatch F0115 from this body with M602_VIEW_SQUARE_D3R. "
    "DUNVIEW.C:F0104:3113-3156 and F0105:3185-3247 define C10-transparent "
    "native and flipped bitmap blits. DUNVIEW.C:F0115:4547-4581 describes "
    "the object/creature/projectile/explosion pass and 5668-5671 documents "
    "the negative-row projectile guard. DUNVIEW.C:F0128:8478-8508 dispatches "
    "D3L2/D3R2 first, then F0116/F0117, then D3C and the D2L2/D2R2 pair; "
    "this gate asserts no D2L2/D0R2 mixed route. DUNGEON.C:F0163/F0164 "
    "lines 1769-1840 and F0172:2466-2523 anchor thing-list and aspect "
    "routing. DEFS.H:2088 C10; DEFS.H:2443/2450, 2582-2583, 2603-2604, "
    "2610-2611, 2662, 2676-2677, 4139-4153, 4197-4198 cover the requested "
    "neighbor matrix; exact F0116/F0117 constants are DEFS.H:2441/2448 "
    "C00/C07, DEFS.H:2608-2609 M601/M602, DEFS.H:4141/4143/4154/4156 "
    "stairs zones, and DEFS.H:4199/4201 floor-pit zones.";

static const DM1_V1_D0L2D0R2StairsPitSpecPc34 s_specs[] = {
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D0L2 alias over F0116 D3L stairs-up front dispatch",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "DUNVIEW.C:F0116:6375-6385",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2441 C00; 4141 C802; 2608 M601; 2676 C0x3421; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, -1, 30, 19, 0, 108, 108, 802, 1, 12, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D0L2 alias over F0116 D3L stairs-down front dispatch",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "DUNVIEW.C:F0116:6386-6405",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2448 C07; 4154 C815; 2608 M601; 2676 C0x3421; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, -1, 30, 19, 7, 115, 108, 815, 1, 12, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D0L2 alias over F0116 D3L open-pit dispatch",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "DUNVIEW.C:F0116:6461-6472",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2333 M754; 4199 C852; 2608 M601; 2676 C0x3421; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, -1, 30, 2, 50, 50, 108, 852, 1, 12, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34,
        "D0L2 alias over F0116 D3L visible-pit follow-up",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "DUNVIEW.C:F0116:6461-6480",
        "DUNVIEW.C:F0116:6461-6480 no visible-pit bitmap",
        "DEFS.H:4199 C852; 2608 M601; 2676 C0x3421; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, -1, 30, 2, -1, -1, 108, 852, 1, 12, 0x0021, 0x3421, 10,
        false, false, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D0R2 alias over F0117 D3R stairs-up front dispatch",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "DUNVIEW.C:F0117:6514-6524",
        "DUNVIEW.C:F0105:3185-3247",
        "DEFS.H:2441 C00; 4143 C804; 2609 M602; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, 1, 40, 19, 0, 108, 108, 804, 2, 13, 0x0021, 0x4312, 10,
        false, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D0R2 alias over F0117 D3R stairs-down front dispatch",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "DUNVIEW.C:F0117:6525-6544",
        "DUNVIEW.C:F0105:3185-3247",
        "DEFS.H:2448 C07; 4156 C817; 2609 M602; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, 1, 40, 19, 7, 115, 108, 817, 2, 13, 0x0021, 0x4312, 10,
        false, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D0R2 alias over F0117 D3R open-pit dispatch",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "DUNVIEW.C:F0117:6603-6614",
        "DUNVIEW.C:F0105:3185-3247",
        "DEFS.H:2333 M754; 4201 C854; 2609 M602; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, 1, 40, 2, 50, 50, 108, 854, 2, 13, 0x0021, 0x4312, 10,
        false, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34,
        "D0R2 alias over F0117 D3R visible-pit follow-up",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "DUNVIEW.C:F0117:6603-6622",
        "DUNVIEW.C:F0117:6603-6622 no visible-pit bitmap",
        "DEFS.H:4201 C854; 2609 M602; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163/F0164:1769-1840",
        3, 3, 1, 40, 2, -1, -1, 108, 854, 2, 13, 0x0021, 0x4312, 10,
        false, false, true, true, true, true, true, true, false
    }
};

size_t dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D0L2D0R2StairsPitSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D0L2D0R2StairsPitSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D0L2D0R2StairsPitSidePc34 side,
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return NULL;
}

const DM1_V1_D0L2D0R2StairsPitAnchorPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count)
{
    if (count) *count = sizeof(s_anchors) / sizeof(s_anchors[0]);
    return s_anchors;
}

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D0L2D0R2StairsPitPixelInputPc34 *input,
    DM1_V1_D0L2D0R2StairsPitPixelResultPc34 *out)
{
    const DM1_V1_D0L2D0R2StairsPitSpecPc34 *spec;
    size_t byte_count;
    size_t row;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!input) return false;
    if (!input->contract_only || input->real_asset_claim) return false;
    if (!input->source || !input->destination) return false;
    if (input->row_width == 0 || input->height == 0) return false;
    if (input->destination_stride < input->row_width) return false;
    if (input->height > ((size_t)-1) / input->row_width) return false;
    if (input->height > ((size_t)-1) / input->destination_stride) return false;

    spec = dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_pc34(
        input->side, input->route);
    if (!spec || spec->native_bitmap_index < 0) return false;

    byte_count = input->row_width * input->height;
    if (input->source_len < byte_count ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    out->contract_only = true;
    out->real_asset_claim = false;
    out->flipped_horizontally = spec->uses_f0105_flipped;
    out->row_width = input->row_width;
    out->height = input->height;
    out->byte_count = byte_count;
    out->destination_stride = input->destination_stride;
    out->first_source_byte = input->source[0];
    out->last_source_byte = input->source[byte_count - 1];
    out->spec = spec;

    for (row = 0; row < input->height; ++row) {
        size_t column;
        const size_t source_row = row * input->row_width;
        const size_t destination_row = row * input->destination_stride;

        for (column = 0; column < input->row_width; ++column) {
            const size_t source_column = spec->uses_f0105_flipped ?
                input->row_width - 1 - column : column;
            const uint8_t pixel = input->source[source_row + source_column];
            if (pixel == (uint8_t)spec->transparent_color) {
                ++out->transparent_skips;
                out->transparent_skip_seen = true;
                continue;
            }
            input->destination[destination_row + column] = pixel;
            ++out->writes;
            out->wrote_any = true;
        }
    }

    out->first_destination_byte = input->destination[0];
    out->last_destination_byte =
        input->destination[(input->height - 1) * input->destination_stride +
                           input->row_width - 1];
    out->ok = true;
    return true;
}

const char *dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34(void)
{
    return s_source_evidence;
}
