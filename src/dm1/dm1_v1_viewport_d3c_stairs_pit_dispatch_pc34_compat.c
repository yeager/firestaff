#include "dm1_v1_viewport_d3c_stairs_pit_dispatch_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source lock. ReDMCSB anchors:
 * - DUNVIEW.C F0118:6666-6696 dispatches D3C stairs through F0104, then
 *   leaves the stairs dispatch arm.
 * - DUNVIEW.C F0118:6748-6763 dispatches D3C open pits through F0104.
 * - DUNVIEW.C F0104:3113-3156 resolves the native bitmap/zone and blits
 *   with C10 transparency.
 * - DUNVIEW.C F0096:2517-2518 seeds G0079 from the current map wallset.
 * - DEFS.H:2442-2449 and 4139-4155 bind D3C stair slots/zones in this
 *   ReDMCSB checkout. This file makes no real-asset pixel parity claim.
 */

static const DM1_V1_D3CStairsPitEvidencePc34 s_evidence[] = {
    {
        DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34,
        "D3C stairs-up front dispatch",
        "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6666-6676",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2442 C01_STAIRS_BITMAP_UP_FRONT_D3C; "
        "DEFS.H:4142 C803_ZONE_STAIRS_UP_FRONT_D3C; "
        "DEFS.H:2607 M600_VIEW_SQUARE_D3C; DEFS.H:2088 C10_COLOR_FLESH",
        DM1_V1_D3C_STAIRS_PIT_PC34_STAIRS_UP_SLOT_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_VIEW_SQUARE_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_CELL_ORDER_OPEN,
        DM1_V1_D3C_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        false
    },
    {
        DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34,
        "D3C stairs-down front dispatch",
        "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6677-6696",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2449 C08_STAIRS_BITMAP_DOWN_FRONT_D3C; "
        "DEFS.H:4155 C816_ZONE_STAIRS_DOWN_FRONT_D3C; "
        "DEFS.H:2607 M600_VIEW_SQUARE_D3C; DEFS.H:2088 C10_COLOR_FLESH",
        DM1_V1_D3C_STAIRS_PIT_PC34_STAIRS_DOWN_SLOT_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_VIEW_SQUARE_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_CELL_ORDER_OPEN,
        DM1_V1_D3C_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        false
    },
    {
        DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34,
        "D3C open-pit dispatch",
        "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6748-6763",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2334 M755_GRAPHIC_FLOOR_PIT_D3C; "
        "DEFS.H:4200 C853_ZONE_FLOORPIT_D3C; "
        "DEFS.H:2607 M600_VIEW_SQUARE_D3C; DEFS.H:2088 C10_COLOR_FLESH",
        DM1_V1_D3C_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_VIEW_SQUARE_D3C,
        DM1_V1_D3C_STAIRS_PIT_PC34_CELL_ORDER_OPEN,
        DM1_V1_D3C_STAIRS_PIT_PC34_TRANSPARENT_COLOR,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        false
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset pixel parity claim. "
    "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6666-6676 routes "
    "D3C stairs-up through F0104 with G0079_ai_StairsNativeBitmapIndices[C01] "
    "and C803_ZONE_STAIRS_UP_FRONT_D3C; "
    "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6677-6696 routes "
    "D3C stairs-down through F0104 with G0079_ai_StairsNativeBitmapIndices[C08] "
    "and C816_ZONE_STAIRS_DOWN_FRONT_D3C; "
    "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6748-6763 routes "
    "open D3C pit through F0104 with M755_GRAPHIC_FLOOR_PIT_D3C and "
    "C853_ZONE_FLOORPIT_D3C before open ordering C0x3421 at 6811-6814; "
    "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156 "
    "uses F0630/F0635 then F0132 with C10_COLOR_FLESH transparency; "
    "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2517-2518 "
    "loads the C018 stairs range from wallset * M647 + M645; "
    "DEFS.H:2442/2449 D3C slots, DEFS.H:4142/4155 D3C zones, "
    "DEFS.H:2334/4200 pit graphic/zone, DEFS.H:2607 D3C view square, "
    "DEFS.H:2088 C10. This D3C stairs/pit dispatch contract excludes "
    "F0107 wall ornament, F0108 metadata, F0111 door, F0115 thing pass, "
    "and F0128 post-D3C wall-followup writes.";

static size_t evidence_count(void)
{
    return sizeof(s_evidence) / sizeof(s_evidence[0]);
}

const DM1_V1_D3CStairsPitEvidencePc34 *
dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_pc34(size_t *count)
{
    if (count) {
        *count = evidence_count();
    }
    return s_evidence;
}

const DM1_V1_D3CStairsPitEvidencePc34 *
dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
    DM1_V1_D3CStairsPitRolePc34 role)
{
    size_t i;

    for (i = 0; i < evidence_count(); ++i) {
        if (s_evidence[i].role == role) {
            return &s_evidence[i];
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(
    const DM1_V1_D3CStairsPitDispatchInputPc34 *input,
    DM1_V1_D3CStairsPitDispatchResultPc34 *out)
{
    const DM1_V1_D3CStairsPitEvidencePc34 *evidence;
    DM1_V1_D3CStairsPitRolePc34 role;

    if (!out) return false;
    memset(out, 0, sizeof(*out));

    if (!input) return false;
    if (!input->contract_only || input->real_asset_claim) return false;

    switch (input->element) {
    case DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT:
        role = input->stairs_up ?
            DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34 :
            DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34;
        break;

    case DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_PIT:
        if (input->pit_or_teleporter_visible) {
            out->unsupported_element = true;
            out->contract_only = true;
            out->real_asset_claim = false;
            out->ok = true;
            return true;
        }
        role = DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34;
        break;

    default:
        out->unsupported_element = true;
        out->contract_only = true;
        out->real_asset_claim = false;
        out->ok = true;
        return true;
    }

    evidence = dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(role);
    if (!evidence) return false;

    out->contract_only = true;
    out->real_asset_claim = false;
    out->role = role;
    out->native_bitmap_slot_or_graphic = evidence->native_bitmap_slot_or_graphic;
    out->first_stairs_graphic_index = DM1_V1_D3C_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC;
    out->native_bitmap_index = role == DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34 ?
        evidence->native_bitmap_slot_or_graphic :
        DM1_V1_D3C_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC +
        evidence->native_bitmap_slot_or_graphic;
    out->zone_index = evidence->zone_index;
    out->view_square_index = evidence->view_square_index;
    out->cell_order = evidence->cell_order;
    out->used_f0104 = evidence->uses_f0104;
    out->used_f0107 = evidence->uses_f0107;
    out->used_f0108_metadata = evidence->uses_f0108_metadata;
    out->used_f0111 = evidence->uses_f0111;
    out->used_f0115_thing_pass = evidence->uses_f0115_thing_pass;
    out->used_f0128_wall_followup_writes = evidence->uses_f0128_wall_followup_writes;
    out->evidence = evidence;
    out->ok = true;
    return true;
}

bool dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(
    const DM1_V1_D3CStairsPitBlitInputPc34 *input,
    DM1_V1_D3CStairsPitBlitResultPc34 *out)
{
    const DM1_V1_D3CStairsPitEvidencePc34 *evidence;
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

    evidence = dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
        input->role);
    if (!evidence) return false;

    byte_count = input->row_width * input->height;
    if (input->source_len < byte_count ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    out->contract_only = true;
    out->real_asset_claim = false;
    out->row_width = input->row_width;
    out->height = input->height;
    out->byte_count = byte_count;
    out->destination_stride = input->destination_stride;
    out->first_source_byte = input->source[0];
    out->last_source_byte = input->source[byte_count - 1];
    out->evidence = evidence;

    /*
     * ReDMCSB DUNVIEW.C:3141-3151 passes C10_COLOR_FLESH to F0132.
     * This synthetic fixture only proves the transparent-byte contract.
     */
    for (row = 0; row < input->height; ++row) {
        size_t column;
        const size_t source_row = row * input->row_width;
        const size_t destination_row = row * input->destination_stride;

        for (column = 0; column < input->row_width; ++column) {
            const uint8_t pixel = input->source[source_row + column];
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

const char *dm1_v1_viewport_d3c_stairs_pit_dispatch_source_evidence_pc34(void)
{
    return s_source_evidence;
}
