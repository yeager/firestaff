#include "dm1/dm1_v1_viewport_f0099_row_local_flip_pc34_compat.h"

#include <string.h>

#define F0099_ANCHOR "DUNVIEW.C:F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal:3018-3075"

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal:3018-3075 copies
 * source bytes to the destination and invokes F0130 with the caller-supplied
 * byte width and height. The callers below provide the PC34 row width/height
 * contracts guarded by this synthetic, contract-only slice.
 */
static const DM1_V1_F0099RowLocalFlipEvidencePc34 s_evidence[] = {
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_F1000_DIRECT_PC34,
        "F1000 direct paired bitmap",
        "L2426_puc_",
        "L2425_puc_",
        F0099_ANCHOR,
        "DUNVIEW.C:F1000_:2092-2092",
        0,
        0,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_F1000_VIEWPORT_SCRATCH_PC34,
        "F1000 viewport scratch bitmap",
        "L2426_puc_",
        "G0296_puc_Bitmap_Viewport",
        F0099_ANCHOR,
        "DUNVIEW.C:F1000_:2101-2101",
        0,
        0,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_DOOR_FRAME_D1C_PC34,
        "Door-frame D1C left to right pair",
        "G0708_puc_Bitmap_WallSet_DoorFrameLeft_D1C",
        "G0710_puc_Bitmap_WallSet_DoorFrameRight_D1C",
        F0099_ANCHOR,
        "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2205-2205",
        16,
        94,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3L2_TO_D3R2_WALL_PC34,
        "D3L2 wall to D3R2 wall",
        "G0697_puc_Bitmap_WallSet_Wall_D3L2",
        "G0696_puc_Bitmap_WallSet_Wall_D3R2",
        F0099_ANCHOR,
        "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2206-2206",
        8,
        49,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3C_WALL_PC34,
        "D3LCR native wall to D3C scratch flip",
        "G0095_puc_Bitmap_WallD3LCR_Native",
        "G0074_puc_Bitmap_Temporary",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2389-2389",
        64,
        51,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D2C_WALL_PC34,
        "D2LCR native wall to D2C scratch flip",
        "G0096_puc_Bitmap_WallD2LCR_Native",
        "G0074_puc_Bitmap_Temporary",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2397-2397",
        72,
        71,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D1LCR_WALL_PC34,
        "D1LCR native wall to flipped wall",
        "G0097_puc_Bitmap_WallD1LCR_Native",
        "G0092_puc_Bitmap_WallD1LCR_Flipped",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2406-2406",
        128,
        111,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0L_TO_D0R_WALL_PC34,
        "D0L native wall to D0R flipped wall",
        "G0098_puc_Bitmap_WallD0L_Native",
        "G0094_puc_Bitmap_WallD0R_Flipped",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2407-2407",
        16,
        136,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0R_TO_D0L_WALL_PC34,
        "D0R native wall to D0L flipped wall",
        "G0099_puc_Bitmap_WallD0R_Native",
        "G0093_puc_Bitmap_WallD0L_Flipped",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2408-2408",
        16,
        136,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4052_FLOOR_PC34,
        "G4052 floor native to flipped floor",
        "G4052_puc_Bitmap_Floor_Native",
        "G4050_puc_Bitmap_Floor_Flipped",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2411-2411",
        112,
        70,
        true,
        false
    },
    {
        DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4053_CEILING_PC34,
        "G4053 ceiling native to flipped ceiling",
        "G4053_puc_Bitmap_Ceiling_Native",
        "G4051_puc_Bitmap_Ceiling_Flipped",
        F0099_ANCHOR,
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2412-2412",
        112,
        29,
        true,
        false
    }
};

const DM1_V1_F0099RowLocalFlipEvidencePc34 *
dm1_v1_viewport_f0099_row_local_flip_evidence_pc34(size_t *count)
{
    if (count) {
        *count = sizeof(s_evidence) / sizeof(s_evidence[0]);
    }
    return s_evidence;
}

const DM1_V1_F0099RowLocalFlipEvidencePc34 *
dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
    DM1_V1_F0099RowLocalFlipRolePc34 role)
{
    size_t i;
    for (i = 0; i < sizeof(s_evidence) / sizeof(s_evidence[0]); ++i) {
        if (s_evidence[i].role == role) return &s_evidence[i];
    }
    return NULL;
}

bool dm1_v1_viewport_f0099_row_local_flip_pc34_compat(
    const DM1_V1_F0099RowLocalFlipStatePc34 *state,
    DM1_V1_F0099RowLocalFlipResultPc34 *out)
{
    size_t byte_count;
    size_t row;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->source_evidence = dm1_v1_viewport_f0099_row_local_flip_source_evidence_pc34();

    if (!state || !state->source || !state->destination) return false;
    if (!state->contract_only || state->real_asset_claim) return false;
    if (state->row_width == 0 || state->height == 0) return false;
    if (state->height > ((size_t)-1) / state->row_width) return false;

    byte_count = state->row_width * state->height;
    if (state->source_len < byte_count || state->destination_len < byte_count) {
        return false;
    }

    out->contract_only = true;
    out->real_asset_claim = false;
    out->in_place = state->source == state->destination;
    out->row_width = state->row_width;
    out->height = state->height;
    out->byte_count = byte_count;
    out->first_source_byte = state->source[0];
    out->last_source_byte = state->source[byte_count - 1];

    if (out->in_place) {
        for (row = 0; row < state->height; ++row) {
            size_t left = row * state->row_width;
            size_t right = left + state->row_width - 1;
            while (left < right) {
                uint8_t temp = state->destination[left];
                state->destination[left] = state->destination[right];
                state->destination[right] = temp;
                ++left;
                --right;
            }
        }
    } else {
        for (row = 0; row < state->height; ++row) {
            size_t column;
            size_t row_offset = row * state->row_width;
            for (column = 0; column < state->row_width; ++column) {
                state->destination[row_offset + column] =
                    state->source[row_offset + state->row_width - 1 - column];
            }
        }
    }

    out->first_destination_byte = state->destination[0];
    out->last_destination_byte = state->destination[byte_count - 1];
    out->rows_flipped = state->height;
    out->ok = true;
    return true;
}

const char *dm1_v1_viewport_f0099_row_local_flip_source_evidence_pc34(void)
{
    return
        "contract_only=1; no real-asset parity claim. "
        "DUNVIEW.C:F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal:3018-3075 "
        "copies P0101 to P0102 then flips P0102 with P0103_ui_ByteWidth and "
        "P0104_ui_Height. Callers: DUNVIEW.C:F1000_:2092-2092, "
        "DUNVIEW.C:F1000_:2101-2101, "
        "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2205-2206, "
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2389-2412.";
}
