#include "dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source lock. ReDMCSB anchors:
 * - DUNVIEW.C F0119:6900-7049 draws the D2L stairs-front and pit branches
 *   through F0104, then falls through F0108/F0112/F0115 follow-up.
 * - DUNVIEW.C F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7051-7220 mirrors those
 *   branches through F0105 flipped and carries the CPSF callout in its name.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 provide the native and
 *   horizontally flipped C10-transparent bitmap contracts. The task's
 *   requested F0105 range 3158-3188 reaches the verified F0105 header at
 *   3185-3188 after the preceding PC media helper.
 * - DUNVIEW.C F0115:4547-4581 documents the object/creature/projectile/
 *   explosion pass reached at F0119 line 7031 and F0120 line 7224.
 * - DUNVIEW.C F0128:8503-8517 draws D2L2/D2R2 wall-side squares first,
 *   then follows with F0119/F0120 D2L/D2R at the second perspective.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905+, and F0172:2466-2523
 *   preserve thing-list and square-aspect map coordinate contracts.
 */

static const DM1_V1_D2L2D2R2StairsPitAnchorPc34 s_anchors[] = {
    {
        "d2l_dispatch", "DUNVIEW.C", "F0119_DUNGEONVIEW_DrawSquareD2L",
        6900, 7049,
        "F0172 aspect switch; C19 and C02 route through F0104 before F0115."
    },
    {
        "d2r_dispatch", "DUNVIEW.C", "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        7051, 7220,
        "F0172 aspect switch; C19 and C02 route through F0105 flipped."
    },
    {
        "native_blit", "DUNVIEW.C",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        3113, 3156,
        "Native bitmap path uses C10_COLOR_FLESH transparency."
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
        "Follow-up pass after floor/ceiling work for non-return branches."
    },
    {
        "projectile_negative_row_guard", "DUNVIEW.C", "F0115 row guard",
        5668, 5671,
        "Negative G2028 rows skip projectile processing."
    },
    {
        "main_draw_order", "DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF",
        8503, 8517,
        "D2L2/D2R2 wall-side writes precede D2L/D2R stairs/pit dispatch."
    },
    {
        "thing_link", "DUNGEON.C", "F0163_DUNGEON_LinkThingToList",
        1769, 1838,
        "Square thing-list coordinate lineage before aspect dispatch."
    },
    {
        "thing_unlink", "DUNGEON.C", "F0164_DUNGEON_UnlinkThingFromList",
        1840, 1905,
        "Square thing-list removal keeps map coordinate lineage."
    },
    {
        "aspect", "DUNGEON.C", "F0172_DUNGEON_SetSquareAspect",
        2466, 2523,
        "F0119/F0120 read square aspect from direction/map X/map Y."
    },
    {
        "defs", "DEFS.H", "constants",
        2088, 4207,
        "C10, C02/C09, M604/M605, C0x0021/C0x3421, D2 zones."
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim. "
    "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:6900-7049 calls "
    "F0172_DUNGEON_SetSquareAspect(direction,mapX,mapY), switches C19 "
    "stairs-front and C02 pit, routes D2L stairs-up through F0104 with "
    "G0079[C02]/C805, stairs-down through F0104 with G0079[C09]/C818, "
    "open pit through F0104 with M756/C855, and invisible pit with "
    "M762/C855. DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7051-7220 "
    "mirrors those native choices through F0105 flipped with C807, C820, "
    "and C857. F0104 is DUNVIEW.C:3113-3156; F0105 starts at "
    "DUNVIEW.C:3185-3247 after the requested 3158-3188 header range. "
    "DEFS.H:2443/2450 binds C02/C09; DEFS.H:2582-2583 binds legacy "
    "M604/M605 and DEFS.H:2603-2604 binds the PC34 M604/M605 indices; "
    "DEFS.H:2662 binds C0x0021 and DEFS.H:2676 binds the open follow-up "
    "C0x3421 order; DEFS.H:4144/4146/4157/4159/4202/4204 binds D2 zones; "
    "DEFS.H:2088 binds C10 transparency. F0119 line 7031 and F0120 line "
    "7224 follow stairs/pit/corridor branches with F0115 using M604/M605. "
    "DUNVIEW.C:F0128:8503-8517 draws D2L2/D2R2 wall-side squares before "
    "the D2L/D2R stairs/pit follow-up writes. DUNGEON.C:F0163:1769-1838, "
    "F0164:1840-1905, and F0172:2466-2523 anchor thing/aspect routing. "
    "This synthetic gate makes no live DOSBox, main-loop, or game-data claim.";

static const DM1_V1_D2L2D2R2StairsPitSpecPc34 s_specs[] = {
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D2L second-perspective stairs-up front dispatch",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:6912-6924",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2443 C02; 4144 C805; 2582/2603 M604; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, -1, -2, 8, 10, 19, 2, 110, 108, 805, 4, 7, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, false, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D2L second-perspective stairs-down front dispatch",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:6925-6942",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2450 C09; 4157 C818; 2582/2603 M604; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, -1, -2, 8, 10, 19, 9, 117, 108, 818, 4, 7, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, false, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D2L second-perspective open-pit dispatch",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:7005-7014",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2335 M756; 4202 C855; 2582/2603 M604; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, -1, -2, 8, 10, 2, 52, 52, 108, 855, 4, 7, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, false, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D2L second-perspective invisible-pit dispatch",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:7005-7014",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2341 M762; 4202 C855; 2582/2603 M604; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, -1, -2, 8, 10, 2, 58, 58, 108, 855, 4, 7, 0x0021, 0x3421, 10,
        true, false, true, true, true, true, true, false, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D2R second-perspective stairs-up front mirrored dispatch",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7063-7075",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2443 C02; 4146 C807; 2583/2604 M605; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, 1, 2, 9, 11, 19, 2, 110, 108, 807, 5, 8, 0x0021, 0x3421, 10,
        false, true, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D2R second-perspective stairs-down front mirrored dispatch",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7076-7093",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2450 C09; 4159 C820; 2583/2604 M605; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, 1, 2, 9, 11, 19, 9, 117, 108, 820, 5, 8, 0x0021, 0x3421, 10,
        false, true, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D2R second-perspective open-pit mirrored dispatch",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7198-7207",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2335 M756; 4204 C857; 2583/2604 M605; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, 1, 2, 9, 11, 2, 52, 52, 108, 857, 5, 8, 0x0021, 0x3421, 10,
        false, true, true, true, true, true, true, true, true, false
    },
    {
        DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
        DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D2R second-perspective invisible-pit mirrored dispatch",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7198-7207",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2341 M762; 4204 C857; 2583/2604 M605; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        2, 2, 1, 2, 9, 11, 2, 58, 58, 108, 857, 5, 8, 0x0021, 0x3421, 10,
        false, true, true, true, true, true, true, true, true, false
    }
};

size_t dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2L2D2R2StairsPitSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D2L2D2R2StairsPitSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D2L2D2R2StairsPitSidePc34 side,
    DM1_V1_D2L2D2R2StairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return NULL;
}

const DM1_V1_D2L2D2R2StairsPitAnchorPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count)
{
    if (count) {
        *count = sizeof(s_anchors) / sizeof(s_anchors[0]);
    }
    return s_anchors;
}

bool dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D2L2D2R2StairsPitPixelInputPc34 *input,
    DM1_V1_D2L2D2R2StairsPitPixelResultPc34 *out)
{
    const DM1_V1_D2L2D2R2StairsPitSpecPc34 *spec;
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

    spec = dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_pc34(
        input->side, input->route);
    if (!spec) return false;

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

const char *dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_source_evidence_pc34(void)
{
    return s_source_evidence;
}
