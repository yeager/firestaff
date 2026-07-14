#include "firestaff/dm1/v1/f0461_start_allocate_flipped_wall_bitmaps_pc34_compat.h"

#include <string.h>

/*
 * F0096:2389-2412 supplies F0099 with these source/destination pairs.
 * Their buffers are allocated earlier by F0461 and remain derived bitmaps,
 * rather than GRAPHICS.DAT records.
 */
static const DM1_V1_F0461FlippedWallBitmapPlanPc34 s_plans[] = {
    {
        DM1_V1_F0461_FLIPPED_WALL_D3LCR_PC34,
        "G0095_puc_Bitmap_WallD3LCR_Native",
        "G0090_puc_Bitmap_WallD3LCR_Flipped",
        64U, 51U, 64U * 51U
    },
    {
        DM1_V1_F0461_FLIPPED_WALL_D2LCR_PC34,
        "G0096_puc_Bitmap_WallD2LCR_Native",
        "G0091_puc_Bitmap_WallD2LCR_Flipped",
        72U, 71U, 72U * 71U
    },
    {
        DM1_V1_F0461_FLIPPED_WALL_D1LCR_PC34,
        "G0097_puc_Bitmap_WallD1LCR_Native",
        "G0092_puc_Bitmap_WallD1LCR_Flipped",
        128U, 111U, 128U * 111U
    },
    {
        DM1_V1_F0461_FLIPPED_WALL_D0L_PC34,
        "G0099_puc_Bitmap_WallD0R_Native",
        "G0093_puc_Bitmap_WallD0L_Flipped",
        16U, 136U, 16U * 136U
    },
    {
        DM1_V1_F0461_FLIPPED_WALL_D0R_PC34,
        "G0098_puc_Bitmap_WallD0L_Native",
        "G0094_puc_Bitmap_WallD0R_Flipped",
        16U, 136U, 16U * 136U
    }
};

bool DM1_V1_F0461_START_AllocateFlippedWallBitmaps_Pc34Compat(
    DM1_V1_F0461FlippedWallBitmapPlanPc34 *plans,
    size_t plan_capacity,
    DM1_V1_F0461AllocateFlippedWallBitmapsResultPc34 *out)
{
    size_t i;
    size_t total = 0;

    if (out) {
        memset(out, 0, sizeof(*out));
        out->source_evidence =
            DM1_V1_F0461_START_AllocateFlippedWallBitmaps_SourceEvidencePc34();
    }
    if (!plans || plan_capacity < DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34) {
        return false;
    }

    memcpy(plans, s_plans, sizeof(s_plans));
    for (i = 0; i < DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34; ++i) {
        total += s_plans[i].allocation_byte_count;
    }
    if (out) {
        out->allocation_count = DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34;
        out->total_allocation_byte_count = total;
        out->derives_by_horizontal_flip = true;
        out->contains_graphic_bytes = false;
    }
    return true;
}

const char *DM1_V1_F0461_START_AllocateFlippedWallBitmaps_SourceEvidencePc34(void)
{
    return
        "ReDMCSB ATARIST.H:223 declares "
        "F0461_START_AllocateFlippedWallBitmaps; "
        "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:"
        "2389-2412 derives G0090-G0094 from G0095-G0099 through "
        "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal; "
        "DUNVIEW.C:F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal:3018-3075 "
        "uses caller row-byte-width and height. No graphic bytes are supplied.";
}
