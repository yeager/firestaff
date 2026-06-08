#include "dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L2_STAIRS_SIDE_SLOT = 17,       /* ReDMCSB DEFS.H:2454 C17_STAIRS_BITMAP_SIDE_D0L. */
    DM1_D0L2_OPEN_PIT_GRAPHIC = 56,       /* ReDMCSB DEFS.H:2339 M760_GRAPHIC_FLOOR_PIT_D0L. */
    DM1_D0L2_INVISIBLE_PIT_GRAPHIC = 62,  /* ReDMCSB DEFS.H:2345 M766_GRAPHIC_FLOOR_PIT_INVISIBLE_D0L. */
    DM1_D0L2_CEILING_PIT_GRAPHIC = 68,    /* ReDMCSB DUNVIEW.C:8015 C068_GRAPHIC_CEILING_PIT_D0L. */
    DM1_ZONE_STAIRS_SIDE_D0L = 832,       /* ReDMCSB DEFS.H:4169 C832_ZONE_STAIRS_SIDE_D0L. */
    DM1_ZONE_STAIRS_SIDE_D0R = 833,       /* ReDMCSB DEFS.H:4170 C833_ZONE_STAIRS_SIDE_D0R. */
    DM1_ZONE_FLOOR_PIT_D0L = 861,         /* ReDMCSB DEFS.H:4210 C861_ZONE_FLOORPIT_D0L. */
    DM1_ZONE_FLOOR_PIT_D0R = 863,         /* ReDMCSB DEFS.H:4212 C863_ZONE_FLOORPIT_D0R. */
    DM1_ZONE_CEILING_PIT_D0L = 870,       /* ReDMCSB DEFS.H:4219 C870_ZONE_CEILING_PIT_D0L. */
    DM1_ZONE_CEILING_PIT_D0R = 872        /* ReDMCSB DEFS.H:4221 C872_ZONE_CEILING_PIT_D0R. */
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no game-data load. "
    "ReDMCSB DUNVIEW.C:F0125:7960-8062 routes D0L C18 stairs-side through "
    "F0104/G0079[C17]/C832 and returns before ceiling-pit/F0115; its C02 "
    "pit route uses F0104 with M760/M766 and C861, then falls through "
    "C870 ceiling-pit plus F0115(M610,C0x0002). DUNVIEW.C:F0126:8064-8162 "
    "mirrors D0R C18 stairs-side through F0105/G0079[C17]/C833 and returns; "
    "its C02 pit route uses F0105 with M760/M766 and C863, then falls "
    "through C872 ceiling-pit plus F0115(M611,C0x0001). DUNVIEW.C:F0104:"
    "3113-3156 and F0105:3185-3247 define C10 transparent native/flipped "
    "bitmap writes. DUNVIEW.C:F0128:8534-8542 dispatches D0L, D0R, then "
    "D0C. DUNGEON.C:F0172:2466-2523 feeds C18/C02 square aspect routing. "
    "DEFS.H:2088 binds C10; DEFS.H:2454 binds C17; DEFS.H:2596-2598 binds "
    "M609/M610/M611; DEFS.H:2658-2663 binds C0x0001/C0x0002/C0x0021; "
    "DEFS.H:4169-4170 and 4210-4221 bind C832/C833/C861/C863/C870/C872. "
    "This gate is non-duplicative with the D0L2/D0R2 wall gate because "
    "it excludes C00 wall-return, and non-duplicative with the F0115 thing "
    "pass gate because it proves the stairs-side early return too.";

static const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 s_specs[] = {
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34,
        "D0L2 stairs-side via F0125 C18",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "DUNVIEW.C:F0125:7973-7984",
        "DUNVIEW.C:F0104:3113-3156",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_STAIRS_SIDE_PC34,
        DM1_D0L2_STAIRS_SIDE_SLOT,
        DM1_ZONE_STAIRS_SIDE_D0L,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0L,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0L_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0L_PC34,
        30, 0, -1, 10,
        true, false, true, false, false, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D0L2 open pit via F0125 C02",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "DUNVIEW.C:F0125:7985-8006",
        "DUNVIEW.C:F0104:3113-3156",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34,
        DM1_D0L2_OPEN_PIT_GRAPHIC,
        DM1_ZONE_FLOOR_PIT_D0L,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0L,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0L_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0L_PC34,
        30, 0, -1, 10,
        true, false, false, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D0L2 invisible pit via F0125 C02",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "DUNVIEW.C:F0125:7985-8006",
        "DUNVIEW.C:F0104:3113-3156",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34,
        DM1_D0L2_INVISIBLE_PIT_GRAPHIC,
        DM1_ZONE_FLOOR_PIT_D0L,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0L,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0L_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0L_PC34,
        30, 0, -1, 10,
        true, false, false, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34,
        "D0R2 stairs-side via F0126 C18",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "DUNVIEW.C:F0126:8077-8088",
        "DUNVIEW.C:F0105:3185-3247",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_STAIRS_SIDE_PC34,
        DM1_D0L2_STAIRS_SIDE_SLOT,
        DM1_ZONE_STAIRS_SIDE_D0R,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0R,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0R_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0R_PC34,
        40, 0, 1, 10,
        false, true, true, false, false, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
        "D0R2 open pit via F0126 C02",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "DUNVIEW.C:F0126:8089-8116",
        "DUNVIEW.C:F0105:3185-3247",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34,
        DM1_D0L2_OPEN_PIT_GRAPHIC,
        DM1_ZONE_FLOOR_PIT_D0R,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0R,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0R_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0R_PC34,
        40, 0, 1, 10,
        false, true, false, true, true, true, true, false
    },
    {
        DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
        "D0R2 invisible pit via F0126 C02",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "DUNVIEW.C:F0126:8089-8116",
        "DUNVIEW.C:F0105:3185-3247",
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34,
        DM1_D0L2_INVISIBLE_PIT_GRAPHIC,
        DM1_ZONE_FLOOR_PIT_D0R,
        DM1_D0L2_CEILING_PIT_GRAPHIC,
        DM1_ZONE_CEILING_PIT_D0R,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0R_PC34,
        DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0R_PC34,
        40, 0, 1, 10,
        false, true, false, true, true, true, true, false
    }
};

static size_t spec_count(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

void dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_init_context_pc34(
    DM1_V1_D0L2D0R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D0L2D0R2StairsPitSidePc34 side)
{
    if (!context) return;
    memset(context, 0, sizeof(*context));
    context->side = side;
    context->element_class =
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_STAIRS_SIDE_PC34;
    context->contract_only = true;
}

const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_specs_pc34(size_t *count)
{
    if (count) *count = spec_count();
    return s_specs;
}

const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_for_pc34(
    DM1_V1_D0L2D0R2StairsPitSidePc34 side,
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route)
{
    size_t i;

    for (i = 0; i < spec_count(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_resolve_pc34(
    const DM1_V1_D0L2D0R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D0L2D0R2StairsPitDispatchResultPc34 *out)
{
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
    const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!context) return false;
    if (!context->contract_only || context->real_asset_claim) return false;

    out->contract_only = true;
    out->side = context->side;
    out->element_class = context->element_class;

    if (context->element_class ==
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_STAIRS_SIDE_PC34) {
        route = DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34;
    } else if (context->element_class ==
        DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34) {
        route = context->pit_or_teleporter_visible ?
            DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 :
            DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34;
    } else {
        out->unsupported_element = true;
        out->ok = true;
        return true;
    }

    spec = dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_for_pc34(
        context->side, route);
    if (!spec) return false;

    out->route = route;
    out->native_bitmap_slot_or_graphic = spec->native_bitmap_slot_or_graphic;
    out->zone_index = spec->zone_index;
    out->ceiling_pit_graphic = spec->ceiling_pit_graphic;
    out->ceiling_pit_zone = spec->ceiling_pit_zone;
    out->view_square_index = spec->view_square_index;
    out->cell_order = spec->cell_order;
    out->used_f0104_native = spec->uses_f0104_native;
    out->used_f0105_flipped = spec->uses_f0105_flipped;
    out->returned_before_ceiling_pit = spec->returns_before_ceiling_pit;
    out->used_ceiling_pit_tail = spec->uses_ceiling_pit_tail;
    out->used_f0115_thing_pass_tail = spec->uses_f0115_thing_pass_tail;
    out->spec = spec;
    out->ok = true;
    return true;
}

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D0L2D0R2StairsPitPixelRunInputPc34 *input,
    DM1_V1_D0L2D0R2StairsPitPixelRunResultPc34 *out)
{
    const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *spec;
    size_t row;
    size_t byte_count;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!input) return false;
    if (!input->contract_only || input->real_asset_claim) return false;
    if (!input->source || !input->destination) return false;
    if (input->row_width == 0 || input->height == 0) return false;
    if (input->destination_stride < input->row_width) return false;
    if (input->height > ((size_t)-1) / input->row_width) return false;
    if (input->height > ((size_t)-1) / input->destination_stride) return false;

    spec = dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_for_pc34(
        input->side, input->route);
    if (!spec) return false;

    byte_count = input->row_width * input->height;
    if (input->source_len < byte_count ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    out->used_f0104_native = spec->uses_f0104_native;
    out->used_f0105_flipped = spec->uses_f0105_flipped;
    out->spec = spec;

    for (row = 0; row < input->height; ++row) {
        size_t column;
        const size_t source_row = row * input->row_width;
        const size_t destination_row = row * input->destination_stride;

        for (column = 0; column < input->row_width; ++column) {
            const size_t source_column = spec->uses_f0105_flipped ?
                input->row_width - 1 - column : column;
            const uint8_t pixel = input->source[source_row + source_column];
            if (pixel == DM1_V1_D0L2_D0R2_STAIRS_PIT_TRANSPARENT_COLOR_PC34) {
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
