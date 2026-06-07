#include "dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB citation table for this synthetic contract-only gate:
 * - DUNVIEW.C F0104 lines 3113-3156: native bitmap blit with C10.
 * - DUNVIEW.C F0105 lines 3185-3218: horizontal-flip bitmap blit with C10.
 * - DUNVIEW.C F0115 lines 4547-4581 and projectile lines 5668-5671:
 *   the thing pass follows D1L/D1R stairs/pit drawing.
 * - DUNVIEW.C F0122 lines 7391-7557 and F0123 lines 7559-7725:
 *   D1L uses F0104; D1R mirrors with F0105.
 * - DUNVIEW.C F0128 lines 8524-8542: after D1L/D1R, D1C, D0L, D0R, and
 *   D0C are drawn; D0C line 8294 binds M609 and C0x0021.
 * - DUNGEON.C F0163/F0164 lines 1769-1840 and F0172 lines 2466-2523:
 *   square aspect and thing-list map-coordinate handoff.
 * - DEFS.H lines 2088, 2445-2452, 2596-2601, 2659-2666, 4147-4162,
 *   and 4205-4207 bind C10, bitmap slots, view squares, orders, and zones.
 */

static const char *const s_anchor_table[] = {
    "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
    "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3218",
    "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581,5668-5671",
    "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7391-7557",
    "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7559-7725",
    "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8542",
    "DUNGEON.C:F0163/F0164:1769-1840",
    "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
    "DEFS.H:2088/2445-2452/2596-2601/2659-2666/4147-4162/4205-4207"
};

static const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 s_specs[] = {
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D1L2 alias over F0122 D1L stairs-up front",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122:7405-7415",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2445 C04; DEFS.H:4147 C808; DEFS.H:2600 M607; DEFS.H:2664 C0x0032; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0122:7523-7536",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, -1, 10, 19, 4, 112, 108, 808, 4, 0x0032, 10,
        true, false, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D1L2 alias over F0122 D1L stairs-down front",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122:7416-7435",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2452 C11; DEFS.H:4160 C821; DEFS.H:2600 M607; DEFS.H:2664 C0x0032; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0122:7523-7536",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, -1, 10, 19, 11, 119, 108, 821, 4, 0x0032, 10,
        true, false, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D1L2 alias over F0122 D1L open pit",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122:7510-7519",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2337 M758; DEFS.H:4205 C858; DEFS.H:2600 M607; DEFS.H:2664 C0x0032; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0122:7523-7536",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, -1, 10, 2, 54, 54, 108, 858, 4, 0x0032, 10,
        true, false, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D1L2 alias over F0122 D1L invisible pit",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122:7510-7519",
        "DUNVIEW.C:F0104:3113-3156",
        "DEFS.H:2343 M764; DEFS.H:4205 C858; DEFS.H:2600 M607; DEFS.H:2664 C0x0032; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0122:7523-7536",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, -1, 10, 2, 60, 60, 108, 858, 4, 0x0032, 10,
        true, false, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D1R2 alias over F0123 D1R stairs-up front",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123:7573-7583",
        "DUNVIEW.C:F0105:3185-3218",
        "DEFS.H:2445 C04; DEFS.H:4149 C810; DEFS.H:2601 M608; DEFS.H:2666 C0x0041; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0123:7691-7704",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, 1, 20, 19, 4, 112, 108, 810, 5, 0x0041, 10,
        false, true, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D1R2 alias over F0123 D1R stairs-down front",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123:7584-7603",
        "DUNVIEW.C:F0105:3185-3218",
        "DEFS.H:2452 C11; DEFS.H:4162 C823; DEFS.H:2601 M608; DEFS.H:2666 C0x0041; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0123:7691-7704",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, 1, 20, 19, 11, 119, 108, 823, 5, 0x0041, 10,
        false, true, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D1R2 alias over F0123 D1R open pit",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123:7678-7687",
        "DUNVIEW.C:F0105:3185-3218",
        "DEFS.H:2337 M758; DEFS.H:4207 C860; DEFS.H:2601 M608; DEFS.H:2666 C0x0041; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0123:7691-7704",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, 1, 20, 2, 54, 54, 108, 860, 5, 0x0041, 10,
        false, true, true, true, 0, 0x0021, true, true, false
    },
    {
        DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D1R2 alias over F0123 D1R invisible pit",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123:7678-7687",
        "DUNVIEW.C:F0105:3185-3218",
        "DEFS.H:2343 M764; DEFS.H:4207 C860; DEFS.H:2601 M608; DEFS.H:2666 C0x0041; DEFS.H:2088 C10",
        "DUNGEON.C:F0163/F0164:1769-1840; DUNGEON.C:F0172:2466-2523",
        "DUNVIEW.C:F0123:7691-7704",
        "DUNVIEW.C:F0128:8524-8542; DUNVIEW.C:F0127:8294",
        2, 1, 1, 1, 20, 2, 60, 60, 108, 860, 5, 0x0041, 10,
        false, true, true, true, 0, 0x0021, true, true, false
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no game-data load. "
    "DUNVIEW.C:F0122:7391-7557 uses F0172 square aspect, dispatches C19 "
    "stairs-up/down and C02 pit through F0104 with C10 transparency, then "
    "reaches F0115 at 7536 with M607/C0x0032. DUNVIEW.C:F0123:7559-7725 "
    "mirrors D1R through F0105 flipped and reaches F0115 at 7704 with "
    "M608/C0x0041. DUNVIEW.C:F0104:3113-3156 and F0105:3185-3218 define "
    "the native and flipped bitmap contracts. DUNVIEW.C:F0128:8524-8542 "
    "draws D1L, D1R, then D1C/D0L/D0R/D0C; DUNVIEW.C:F0127:8294 anchors "
    "the post-D1 side-lane D0C thing pass with M609_VIEW_SQUARE_D0C and "
    "C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT. DEFS.H:2088 C10; "
    "DEFS.H:2445-2452 C04/C11; DEFS.H:2596-2601 M609/M607/M608; "
    "DEFS.H:2659-2666 C0x0021/C0x0032/C0x0041; DEFS.H:4147-4162 and "
    "4205-4207 bind D1L/D1R stairs/pit zones. DUNGEON.C:F0163/F0164 "
    "lines 1769-1840 preserve thing-list map interaction and F0172:2466-2523 "
    "feeds aspect routing from direction/map coordinates.";

static size_t spec_count(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

void dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_init_context_pc34(
    DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D1L2D1R2StairsPitSidePc34 side)
{
    if (!context) return;
    memset(context, 0, sizeof(*context));
    context->side = side;
    context->element_class =
        DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT;
    context->stairs_up = true;
    context->contract_only = true;
    context->real_asset_claim = false;
}

const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_pc34(size_t *count)
{
    if (count) *count = spec_count();
    return s_specs;
}

const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
    DM1_V1_D1L2D1R2StairsPitSidePc34 side,
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < spec_count(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
    const DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D1L2D1R2StairsPitDispatchResultPc34 *out)
{
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
    const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!context) return false;
    if (!context->contract_only || context->real_asset_claim) return false;

    out->contract_only = true;
    out->real_asset_claim = false;
    out->side = context->side;
    out->direction = context->direction;
    out->map_x = context->map_x;
    out->map_y = context->map_y;
    out->element_class = context->element_class;

    switch (context->element_class) {
    case DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT:
        route = context->stairs_up ?
            DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34 :
            DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34;
        break;
    case DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_PIT:
        route = context->pit_or_teleporter_visible ?
            DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 :
            DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34;
        break;
    default:
        out->unsupported_element = true;
        out->ok = true;
        return true;
    }

    spec = dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
        context->side, route);
    if (!spec) return false;

    out->route = route;
    out->native_bitmap_slot_or_graphic = spec->native_bitmap_slot_or_graphic;
    out->native_bitmap_index = spec->native_bitmap_index;
    out->zone_index = spec->zone_index;
    out->view_square_index = spec->view_square_index;
    out->cell_order = spec->cell_order;
    out->l2_perspective_index = spec->l2_perspective_index;
    out->redmcsb_view_depth = spec->redmcsb_view_depth;
    out->used_f0104_native = spec->uses_f0104_native;
    out->used_f0105_flipped = spec->uses_f0105_flipped;
    out->used_f0115_thing_pass_followup =
        spec->uses_f0115_thing_pass_followup;
    out->follows_with_d1c_d0l_d0r_d0c = spec->follows_with_d1c_d0l_d0r_d0c;
    out->followup_d0c_view_square_index = spec->followup_d0c_view_square_index;
    out->followup_d0c_cell_order = spec->followup_d0c_cell_order;
    out->followup_d0c_uses_f0115 = spec->followup_d0c_uses_f0115;
    out->spec = spec;
    out->ok = true;
    return true;
}

bool dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34 *input,
    DM1_V1_D1L2D1R2StairsPitPixelRunResultPc34 *out)
{
    const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *spec;
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

    spec = dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
        input->side, input->route);
    if (!spec) return false;

    byte_count = input->row_width * input->height;
    if (input->source_len < byte_count ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    out->contract_only = true;
    out->real_asset_claim = false;
    out->used_f0104_native = spec->uses_f0104_native;
    out->used_f0105_flipped = spec->uses_f0105_flipped;
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

const char *dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *const *dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_anchor_table_pc34(
    size_t *count)
{
    if (count) *count = sizeof(s_anchor_table) / sizeof(s_anchor_table[0]);
    return s_anchor_table;
}
