#include "dm1/dm1_v1_viewport_d2c_stair_rail_pc34_compat.h"

#include <string.h>

#define DM1_V1_D2C_STAIR_FIRST_STAIRS_GRAPHIC_PC34 108
#define DM1_V1_D2C_STAIR_WALLSET_GRAPHIC_COUNT_PC34 40
#define DM1_V1_D2C_STAIR_VIEW_SQUARE_D2C_PC34 6
#define DM1_V1_D2C_STAIR_TRANSPARENT_COLOR_PC34 10

/*
 * Contract-only source lock. ReDMCSB anchors:
 * DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7257-7288 dispatches D2C
 * stairs-up/down through F0104 with G0079_ai_StairsNativeBitmapIndices
 * slots C03 and C10 and zones C806/C819. DUNVIEW.C:F0104:3113-3156
 * resolves the native bitmap, resolves the zone, and blits with C10
 * transparency. DUNVIEW.C:F0096:2517-2518 seeds G0079 from the current
 * wallset's stairs graphic range. This file makes no real-asset parity claim.
 */
static const DM1_V1_D2CStairRailEvidencePc34 s_evidence[] = {
    {
        DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34,
        "D2C stairs-up front rail",
        "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7257-7268",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2444 C03_STAIRS_BITMAP_UP_FRONT_D2C; "
        "DEFS.H:4145 C806_ZONE_STAIRS_UP_FRONT_D2C; "
        "DEFS.H:2602 M603_VIEW_SQUARE_D2C; DEFS.H:2088 C10_COLOR_FLESH",
        3,
        806,
        DM1_V1_D2C_STAIR_VIEW_SQUARE_D2C_PC34,
        DM1_V1_D2C_STAIR_TRANSPARENT_COLOR_PC34,
        6,
        4,
        true,
        false
    },
    {
        DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34,
        "D2C stairs-down front rail",
        "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7269-7288",
        "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
        "DEFS.H:2451 C10_STAIRS_BITMAP_DOWN_FRONT_D2C; "
        "DEFS.H:4158 C819_ZONE_STAIRS_DOWN_FRONT_D2C; "
        "DEFS.H:2602 M603_VIEW_SQUARE_D2C; DEFS.H:2088 C10_COLOR_FLESH",
        10,
        819,
        DM1_V1_D2C_STAIR_VIEW_SQUARE_D2C_PC34,
        DM1_V1_D2C_STAIR_TRANSPARENT_COLOR_PC34,
        6,
        4,
        true,
        false
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no real-asset stairs bitmap parity claim. "
    "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7257-7268 routes D2C "
    "stairs-up through G0079_ai_StairsNativeBitmapIndices[C03] and "
    "C806_ZONE_STAIRS_UP_FRONT_D2C; "
    "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7269-7288 routes D2C "
    "stairs-down through G0079_ai_StairsNativeBitmapIndices[C10] and "
    "C819_ZONE_STAIRS_DOWN_FRONT_D2C; "
    "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156 "
    "uses F0630/F0635 then F0132 with C10_COLOR_FLESH transparency; "
    "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2517-2518 "
    "loads the C018 stairs range from wallset * M647 + M645; "
    "DEFS.H:2444/2451 slots, DEFS.H:4145/4158 zones, "
    "DEFS.H:2602 D2C view square, DEFS.H:2088 C10.";

static size_t evidence_count(void)
{
    return sizeof(s_evidence) / sizeof(s_evidence[0]);
}

const DM1_V1_D2CStairRailEvidencePc34 *
dm1_v1_viewport_d2c_stair_rail_evidence_pc34(size_t *count)
{
    if (count) {
        *count = evidence_count();
    }
    return s_evidence;
}

const DM1_V1_D2CStairRailEvidencePc34 *
dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
    DM1_V1_D2CStairRailRolePc34 role)
{
    size_t i;

    for (i = 0; i < evidence_count(); ++i) {
        if (s_evidence[i].role == role) {
            return &s_evidence[i];
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d2c_stair_rail_resolve_pc34(
    const DM1_V1_D2CStairRailResolveInputPc34 *input,
    DM1_V1_D2CStairRailResolveResultPc34 *out)
{
    const DM1_V1_D2CStairRailEvidencePc34 *evidence;

    if (!out) return false;
    memset(out, 0, sizeof(*out));

    if (!input) return false;
    if (!input->contract_only || input->real_asset_claim) return false;
    if (input->wallset_index < 0) return false;

    evidence = dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(input->role);
    if (!evidence) return false;

    /*
     * ReDMCSB DUNVIEW.C:2517-2518 computes the first stairs graphic from
     * wallset * M647_WALL_SET_GRAPHIC_COUNT + M645_GRAPHIC_FIRST_STAIRS and
     * stores consecutive entries into G0079_ai_StairsNativeBitmapIndices.
     */
    out->first_stairs_graphic_index =
        input->wallset_index * DM1_V1_D2C_STAIR_WALLSET_GRAPHIC_COUNT_PC34 +
        DM1_V1_D2C_STAIR_FIRST_STAIRS_GRAPHIC_PC34;
    out->stairs_bitmap_slot = evidence->stairs_bitmap_slot;
    out->native_bitmap_index =
        out->first_stairs_graphic_index + evidence->stairs_bitmap_slot;
    out->zone_index = evidence->zone_index;
    out->view_square_index = evidence->view_square_index;
    out->contract_only = true;
    out->real_asset_claim = false;
    out->evidence = evidence;
    out->ok = true;
    return true;
}

bool dm1_v1_viewport_d2c_stair_rail_blit_pc34(
    const DM1_V1_D2CStairRailBlitInputPc34 *input,
    DM1_V1_D2CStairRailBlitResultPc34 *out)
{
    const DM1_V1_D2CStairRailEvidencePc34 *evidence;
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

    evidence = dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(input->role);
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
     * ReDMCSB DUNVIEW.C:3141-3151 passes the resolved bitmap and viewport
     * zone to F0132 with C10_COLOR_FLESH. BLIT.C:30-31 defines F0132 as a
     * bounded bitmap-region blit, so this contract copies by rows and leaves
     * C10 source bytes unchanged in the destination.
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

const char *dm1_v1_viewport_d2c_stair_rail_source_evidence_pc34(void)
{
    return s_source_evidence;
}
