#include "dm1_v1_viewport_d1l_d1r_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source lock. ReDMCSB anchors:
 * - DUNVIEW.C F0122:7391-7557 dispatches D1L stairs front through F0104
 *   and D1L pits through F0104.
 * - DUNVIEW.C F0123:7559-7725 mirrors the same bitmap choices through
 *   F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally.
 * - DUNVIEW.C F0104/F0105:3113-3188 performs native bitmap blits with
 *   C10 transparency. This fixture makes no real-asset pixel claim.
 * - DUNGEON.C F0172:2466-2523 feeds square aspect from direction/map X/Y.
 */

static const DM1_V1_D1LD1RStairsPitEvidencePc34 s_evidence[] = {
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D1L stairs-up front dispatch",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7405-7415",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2445 C04_STAIRS_BITMAP_UP_FRONT_D1L; "
        "DEFS.H:4147 C808_ZONE_STAIRS_UP_FRONT_D1L; "
        "DEFS.H:2600 M607_VIEW_SQUARE_D1L; DEFS.H:2664 C0x0032; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC +
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true, false, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D1L stairs-down front dispatch",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7416-7435",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2452 C11_STAIRS_BITMAP_DOWN_FRONT_D1L; "
        "DEFS.H:4160 C821_ZONE_STAIRS_DOWN_FRONT_D1L; "
        "DEFS.H:2600 M607_VIEW_SQUARE_D1L; DEFS.H:2664 C0x0032; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC +
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true, false, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D1L open-pit dispatch",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7510-7519",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2337 M758_GRAPHIC_FLOOR_PIT_D1L; "
        "DEFS.H:4205 C858_ZONE_FLOORPIT_D1L; "
        "DEFS.H:2600 M607_VIEW_SQUARE_D1L; DEFS.H:2664 C0x0032; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true, false, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D1L invisible-pit dispatch",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7510-7519",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2343 M764_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L; "
        "DEFS.H:4205 C858_ZONE_FLOORPIT_D1L; "
        "DEFS.H:2600 M607_VIEW_SQUARE_D1L; DEFS.H:2664 C0x0032; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true, false, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
        "D1R stairs-up front mirrored dispatch",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7573-7583",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3158-3188",
        "DEFS.H:2445 C04_STAIRS_BITMAP_UP_FRONT_D1L; "
        "DEFS.H:4149 C810_ZONE_STAIRS_UP_FRONT_D1R; "
        "DEFS.H:2601 M608_VIEW_SQUARE_D1R; DEFS.H:2666 C0x0041; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC +
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        false, true, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
        "D1R stairs-down front mirrored dispatch",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7584-7603",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3158-3188",
        "DEFS.H:2452 C11_STAIRS_BITMAP_DOWN_FRONT_D1L; "
        "DEFS.H:4162 C823_ZONE_STAIRS_DOWN_FRONT_D1R; "
        "DEFS.H:2601 M608_VIEW_SQUARE_D1R; DEFS.H:2666 C0x0041; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC +
        DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        false, true, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D1R open-pit mirrored dispatch",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7678-7687",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3158-3188",
        "DEFS.H:2337 M758_GRAPHIC_FLOOR_PIT_D1L; "
        "DEFS.H:4207 C860_ZONE_FLOORPIT_D1R; "
        "DEFS.H:2601 M608_VIEW_SQUARE_D1R; DEFS.H:2666 C0x0041; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        false, true, false, false, true, false
    },
    {
        DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
        DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D1R invisible-pit mirrored dispatch",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7678-7687",
        "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3158-3188",
        "DEFS.H:2343 M764_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L; "
        "DEFS.H:4207 C860_ZONE_FLOORPIT_D1R; "
        "DEFS.H:2601 M608_VIEW_SQUARE_D1R; DEFS.H:2666 C0x0041; "
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523",
        DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L,
        DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1R,
        DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN,
        DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        false, true, false, false, true, false
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset pixel parity claim. "
    "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7391-7557 calls "
    "F0172_DUNGEON_SetSquareAspect(direction,mapX,mapY), switches C19 "
    "stairs-front and C02 pit, routes D1L stairs-up through F0104 with "
    "G0079[C04]/C808, stairs-down through F0104 with G0079[C11]/C821, "
    "open pit through F0104 with M758/C858, and invisible pit with "
    "M764/C858. DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7559-7725 mirrors "
    "those same native bitmap choices through F0105 flipped with zones "
    "C810, C823, and C860. DEFS.H:1009/1016/1017 binds C02/C18/C19; "
    "DEFS.H:2445/2452 binds C04/C11; DEFS.H:2600-2601 binds M607/M608; "
    "DEFS.H:4147/4149/4160/4162/4205/4207 binds D1L/D1R zones; "
    "DEFS.H:2088 binds C10 transparency. DUNGEON.C:F0163/F0164 preserve "
    "thing-list map coordinates and F0172:2466-2523 reads square aspect "
    "from the supplied direction/mapX/mapY. This synthetic dispatch contract "
    "excludes F0111 door rendering and F0115 thing-pass rendering for the "
    "D1L/D1R stairs/pit bitmap dispatch cases.";

static size_t evidence_count(void)
{
    return sizeof(s_evidence) / sizeof(s_evidence[0]);
}

void M11_GameView_ViewportD1LD1RStairsPitDispatch_InitContextPc34(
    DM1_V1_D1LD1RStairsPitDispatchContextPc34 *context,
    DM1_V1_D1LD1RStairsPitSidePc34 side)
{
    if (!context) return;
    memset(context, 0, sizeof(*context));
    context->side = side;
    context->element_class = DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT;
    context->stairs_up = true;
    context->contract_only = true;
    context->real_asset_claim = false;
}

const DM1_V1_D1LD1RStairsPitEvidencePc34 *
M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidencePc34(size_t *count)
{
    if (count) {
        *count = evidence_count();
    }
    return s_evidence;
}

const DM1_V1_D1LD1RStairsPitEvidencePc34 *
M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
    DM1_V1_D1LD1RStairsPitSidePc34 side,
    DM1_V1_D1LD1RStairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < evidence_count(); ++i) {
        if (s_evidence[i].side == side && s_evidence[i].route == route) {
            return &s_evidence[i];
        }
    }
    return NULL;
}

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(
    const DM1_V1_D1LD1RStairsPitDispatchContextPc34 *context,
    DM1_V1_D1LD1RStairsPitDispatchResultPc34 *out)
{
    DM1_V1_D1LD1RStairsPitRoutePc34 route;
    const DM1_V1_D1LD1RStairsPitEvidencePc34 *evidence;

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
    case DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT:
        route = context->stairs_up ?
            DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34 :
            DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34;
        break;
    case DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT:
        route = context->pit_or_teleporter_visible ?
            DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 :
            DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34;
        break;
    default:
        out->unsupported_element = true;
        out->ok = true;
        return true;
    }

    evidence = M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
        context->side, route);
    if (!evidence) return false;

    out->route = route;
    out->native_bitmap_slot_or_graphic = evidence->native_bitmap_slot_or_graphic;
    out->native_bitmap_index = evidence->native_bitmap_index;
    out->first_stairs_graphic_index =
        DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC;
    out->zone_index = evidence->zone_index;
    out->view_square_index = evidence->view_square_index;
    out->cell_order = evidence->cell_order;
    out->used_f0104 = evidence->uses_f0104;
    out->used_f0105_flipped = evidence->uses_f0105_flipped;
    out->used_f0111 = evidence->uses_f0111;
    out->used_f0115_thing_pass = evidence->uses_f0115_thing_pass;
    out->evidence = evidence;
    out->ok = true;
    return true;
}

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(
    const DM1_V1_D1LD1RStairsPitBlitInputPc34 *input,
    DM1_V1_D1LD1RStairsPitBlitResultPc34 *out)
{
    const DM1_V1_D1LD1RStairsPitEvidencePc34 *evidence;
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

    evidence = M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
        input->side, input->route);
    if (!evidence) return false;

    byte_count = input->row_width * input->height;
    if (input->source_len < byte_count ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    out->contract_only = true;
    out->real_asset_claim = false;
    out->flipped_horizontally = evidence->uses_f0105_flipped;
    out->row_width = input->row_width;
    out->height = input->height;
    out->byte_count = byte_count;
    out->destination_stride = input->destination_stride;
    out->first_source_byte = input->source[0];
    out->last_source_byte = input->source[byte_count - 1];
    out->evidence = evidence;

    for (row = 0; row < input->height; ++row) {
        size_t column;
        const size_t source_row = row * input->row_width;
        const size_t destination_row = row * input->destination_stride;

        for (column = 0; column < input->row_width; ++column) {
            const size_t source_column = evidence->uses_f0105_flipped ?
                input->row_width - 1 - column : column;
            const uint8_t pixel = input->source[source_row + source_column];
            if (pixel == (uint8_t)evidence->transparent_color) {
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

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_AssertPc34(
    DM1_V1_D1LD1RStairsPitAssertResultPc34 *out)
{
    size_t i;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->expected_assertions = 8;
    out->evidence_table_complete = evidence_count() == 8;
    out->d1l_uses_f0104 = true;
    out->d1r_uses_f0105 = true;
    out->no_f0111 = true;
    out->no_f0115_thing_pass = true;

    for (i = 0; i < evidence_count(); ++i) {
        if (s_evidence[i].side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 &&
            !s_evidence[i].uses_f0104) {
            out->d1l_uses_f0104 = false;
        }
        if (s_evidence[i].side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34 &&
            !s_evidence[i].uses_f0105_flipped) {
            out->d1r_uses_f0105 = false;
        }
        if (s_evidence[i].uses_f0111) {
            out->no_f0111 = false;
        }
        if (s_evidence[i].uses_f0115_thing_pass) {
            out->no_f0115_thing_pass = false;
        }
    }

    out->failures =
        (out->evidence_table_complete ? 0 : 1) +
        (out->d1l_uses_f0104 ? 0 : 1) +
        (out->d1r_uses_f0105 ? 0 : 1) +
        (out->no_f0111 ? 0 : 1) +
        (out->no_f0115_thing_pass ? 0 : 1);
    return out->failures == 0;
}

const char *M11_GameView_ViewportD1LD1RStairsPitDispatch_SourceEvidencePc34(void)
{
    return s_source_evidence;
}
