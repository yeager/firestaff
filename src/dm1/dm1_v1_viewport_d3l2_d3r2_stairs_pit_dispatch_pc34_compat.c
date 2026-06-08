#include "dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source lock. ReDMCSB anchors:
 * - DUNVIEW.C F0676:6226-6291 draws the D3L2 stairs-front and pit branches
 *   through F0104, then falls through F0108/F0115 follow-up.
 * - DUNVIEW.C F0677:6293-6358 mirrors those D3R2 branches through F0105.
 * - DUNVIEW.C F0116:6361-6480 and F0117:6500-6622 anchor the neighboring
 *   D3L/D3R wall routes, and F0678:6837-6866 anchors the later D2L2 reuse.
 * - DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 provide the native and
 *   horizontally flipped C10-transparent bitmap contracts. The task's
 *   requested F0105 range 3158-3188 reaches the verified F0105 header at
 *   3185-3188 after the preceding PC media helper.
 * - DUNVIEW.C F0115:4547-4581 documents the object/creature/projectile/
 *   explosion pass reached at F0676 line 6286 and F0677 line 6353.
 * - DUNVIEW.C F0128:8478-8508 draws D3L2/D3R2 before D3L/D3R/D3C and
 *   the following F0678/F0679 D2L2/D2R2 pair; F0127:8294 anchors the
 *   C0x0021 main-wall follow-up order required by this D-row matrix.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905+, and F0172:2466-2523
 *   preserve thing-list and square-aspect map coordinate contracts.
 */

static const DM1_V1_D3L2D3R2StairsPitAnchorPc34 s_anchors[] = {
    {
        "d3l2_dispatch", "DUNVIEW.C", "F0676_DrawD3L2",
        6226, 6291,
        "F0172 aspect switch; C19 and C02 route through F0104 before F0115."
    },
    {
        "d3r2_dispatch", "DUNVIEW.C", "F0677_DrawD3R2",
        6293, 6358,
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
        "d3l_wall_route", "DUNVIEW.C", "F0116_DUNGEONVIEW_DrawSquareD3L",
        6361, 6480,
        "Neighboring D3L route follows the D3L2 column-pair dispatch."
    },
    {
        "d3r_wall_route", "DUNVIEW.C", "F0117_DUNGEONVIEW_DrawSquareD3R",
        6500, 6622,
        "Neighboring D3R route follows the D3R2 column-pair dispatch."
    },
    {
        "d2l2_reuse", "DUNVIEW.C", "F0678_DrawD2L2",
        6837, 6866,
        "Later D2L2 wall helper reuses the D-row column-pair pattern."
    },
    {
        "main_draw_order", "DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF",
        8478, 8508,
        "D3L2/D3R2 dispatch precedes D3L/D3R/D3C and D2L2/D2R2."
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
        "F0676/F0677 read square aspect from direction/map X/map Y."
    },
    {
        "defs", "DEFS.H", "constants",
        2088, 4207,
        "C10, C02/C09 slot lineage, M604/M605 related view IDs, C0x0021/C0x3421, D3 zones."
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim. "
    "DUNVIEW.C:F0676_DrawD3L2:6226-6291 calls "
    "F0172_DUNGEON_SetSquareAspect(direction,mapX,mapY), switches C19 "
    "stairs-front and C02 pit, routes D3L2 stairs-up through F0104 with "
    "M714_NEGGRAPHIC_STAIRS_UP_D3L2/C800, stairs-down through F0104 with "
    "M716_NEGGRAPHIC_STAIRS_DOWN_D3L2/C813, and open pit through F0104 "
    "with C049_GRAPHIC_FLOOR_PIT_D3L2/C850. Visible pits and teleporters "
    "continue to F0108/F0115 without an additional floor-pit bitmap. "
    "DUNVIEW.C:F0677_DrawD3R2:6293-6358 mirrors those native choices "
    "through F0105 flipped with M715/C801, M717/C814, and C851. "
    "DUNVIEW.C:F0116_DUNGEONVIEW_DrawSquareD3L:6361-6480 and "
    "DUNVIEW.C:F0117_DUNGEONVIEW_DrawSquareD3R:6500-6622 anchor the "
    "neighboring D3 wall routes; DUNVIEW.C:F0678_DrawD2L2:6837-6866 is "
    "the later D2L2 wall helper reused by the D-row matrix. F0104 is "
    "DUNVIEW.C:3113-3156; F0105 starts at "
    "DUNVIEW.C:3185-3247 after the requested 3158-3188 header range. "
    "DEFS.H:3674-3677 binds M714/M715/M716/M717; DEFS.H:2443/2450 binds "
    "the related C02/C09 D2 slot lineage and DEFS.H:2582-2583/2603-2604 "
    "binds related M604/M605 view IDs; DEFS.H:2610-2611 binds C14/C15 "
    "D3L2/D3R2 view IDs. DEFS.H:2662 binds C0x0021 and DEFS.H:2676 binds "
    "the C0x3421 open order; D3R2 mirrors with C0x4312 at DEFS.H:2677. "
    "DEFS.H:4139-4153 and 4197-4198 bind D3L2/D3R2 zones; DEFS.H:2088 "
    "binds C10 transparency. F0676 line 6286 and F0677 line 6353 follow "
    "stairs/pit/corridor branches with F0115 using C14/C15. "
    "DUNVIEW.C:F0128:8478-8508 draws D3L2/D3R2 before D3L/D3R/D3C and "
    "the D2L2/D2R2 follow-up writes; DUNVIEW.C:F0127:8294 supplies the "
    "main-wall C0x0021 follow-up anchor. DUNGEON.C:F0163:1769-1838, "
    "F0164:1840-1905, and F0172:2466-2523 anchor thing/aspect routing. "
    "This synthetic gate makes no live DOSBox, main-loop, or game-data claim.";

static const DM1_V1_D3L2D3R2StairsPitSpecPc34 s_specs[] = {
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3L2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D3L2 second-perspective stairs-up front dispatch",
        "F0676_DrawD3L2",
        "DUNVIEW.C:F0676_DrawD3L2:6237-6240",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2443 C02; 4139 C800; 2610 C14; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, -2, -2, -1, 0, 19, -18, -18, 108, 800, 14, 14, 0x0021, 0x3421, 10,
        true, false, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3L2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D3L2 second-perspective stairs-down front dispatch",
        "F0676_DrawD3L2",
        "DUNVIEW.C:F0676_DrawD3L2:6240-6251",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2450 C09; 4152 C813; 2610 C14; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, -2, -2, -1, 0, 19, -20, -20, 108, 813, 14, 14, 0x0021, 0x3421, 10,
        true, false, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3L2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D3L2 second-perspective open-pit dispatch",
        "F0676_DrawD3L2",
        "DUNVIEW.C:F0676_DrawD3L2:6275-6286",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2332 C049; 4197 C850; 2610 C14; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, -2, -2, -1, 0, 2, 49, 49, 108, 850, 14, 14, 0x0021, 0x3421, 10,
        true, false, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3L2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34,
        "D3L2 second-perspective visible-pit follow-up dispatch",
        "F0676_DrawD3L2",
        "DUNVIEW.C:F0676_DrawD3L2:6275-6286",
        "DUNVIEW.C:F0676_DrawD3L2:6275-6286 no visible-pit bitmap",
        "DEFS.H:6275-6286 visible pit follow-up; 4197 C850; 2610 C14; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, -2, -2, -1, 0, 2, -1, -1, 108, 850, 14, 14, 0x0021, 0x3421, 10,
        false, false, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3R2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D3R2 second-perspective stairs-up front mirrored dispatch",
        "F0677_DrawD3R2",
        "DUNVIEW.C:F0677_DrawD3R2:6304-6307",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2443 C02; 4140 C801; 2611 C15; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, 2, 2, -1, 1, 19, -19, -19, 108, 801, 15, 15, 0x0021, 0x4312, 10,
        false, true, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3R2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D3R2 second-perspective stairs-down front mirrored dispatch",
        "F0677_DrawD3R2",
        "DUNVIEW.C:F0677_DrawD3R2:6307-6318",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2450 C09; 4153 C814; 2611 C15; 2662 C0x0021; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, 2, 2, -1, 1, 19, -21, -21, 108, 814, 15, 15, 0x0021, 0x4312, 10,
        false, true, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3R2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D3R2 second-perspective open-pit mirrored dispatch",
        "F0677_DrawD3R2",
        "DUNVIEW.C:F0677_DrawD3R2:6342-6353",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247",
        "DEFS.H:2332 C049; 4198 C851; 2611 C15; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, 2, 2, -1, 1, 2, 49, 49, 108, 851, 15, 15, 0x0021, 0x4312, 10,
        false, true, true, false, true, true, true, false, true, false
    },
    {
        DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3R2_PC34,
        DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34,
        "D3R2 second-perspective visible-pit follow-up mirrored dispatch",
        "F0677_DrawD3R2",
        "DUNVIEW.C:F0677_DrawD3R2:6342-6353",
        "DUNVIEW.C:F0677_DrawD3R2:6342-6353 no visible-pit bitmap",
        "DEFS.H:6342-6353 visible pit follow-up; 4198 C851; 2611 C15; 2677 C0x4312; 2088 C10",
        "DUNGEON.C:F0172:2466-2523; F0163:1769-1838; F0164:1840-1905",
        3, 3, 2, 2, -1, 1, 2, -1, -1, 108, 851, 15, 15, 0x0021, 0x4312, 10,
        false, false, true, false, true, true, true, false, true, false
    }
};

size_t dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D3L2D3R2StairsPitSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D3L2D3R2StairsPitSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D3L2D3R2StairsPitSidePc34 side,
    DM1_V1_D3L2D3R2StairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return NULL;
}

const DM1_V1_D3L2D3R2StairsPitAnchorPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count)
{
    if (count) {
        *count = sizeof(s_anchors) / sizeof(s_anchors[0]);
    }
    return s_anchors;
}

bool dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D3L2D3R2StairsPitPixelInputPc34 *input,
    DM1_V1_D3L2D3R2StairsPitPixelResultPc34 *out)
{
    const DM1_V1_D3L2D3R2StairsPitSpecPc34 *spec;
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

    spec = dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_pc34(
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

const char *dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_source_evidence_pc34(void)
{
    return s_source_evidence;
}
