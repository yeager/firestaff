#include "dm1_v1_floor_pit_pc34_compat.h"

#include <string.h>

/* ReDMCSB DEFS.H lines 2332-2346 define the PC34 floor-pit graphics and
 * lines 4197-4208 define the matching floor-pit zones.  DUNGEON.C F0172
 * line 2629 sets M554 only for MASK0x0008_PIT_OPEN; closed pits render as
 * corridor floor. */
enum {
    DM1_GFX_FLOOR_PIT_D3L2_PC34 = 49,
    DM1_GFX_FLOOR_PIT_D3L_PC34 = 50,
    DM1_GFX_FLOOR_PIT_D3C_PC34 = 51,
    DM1_GFX_FLOOR_PIT_D2L_PC34 = 52,
    DM1_GFX_FLOOR_PIT_D2C_PC34 = 53,
    DM1_GFX_FLOOR_PIT_D1L_PC34 = 54,
    DM1_GFX_FLOOR_PIT_D1C_PC34 = 55,
    DM1_GFX_FLOOR_PIT_D0L_PC34 = 56,
    DM1_GFX_FLOOR_PIT_D0C_PC34 = 57,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D2L_PC34 = 58,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D2C_PC34 = 59,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D1L_PC34 = 60,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D1C_PC34 = 61,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D0L_PC34 = 62,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D0C_PC34 = 63
};

static const DM1_FloorPitRenderPlanPc34 s_floorPitPlans[] = {
    {3, -2, {DM1_GFX_FLOOR_PIT_D3L2_PC34, 0, 0, 0,   66, 22, 10}, {0}, 0},
    {3,  2, {DM1_GFX_FLOOR_PIT_D3L2_PC34, 0, 0, 202, 66, 22, 10}, {0}, 0},
    {3, -1, {DM1_GFX_FLOOR_PIT_D3L_PC34,  0, 0, 4,   65, 78, 8},  {0}, 0},
    {3,  0, {DM1_GFX_FLOOR_PIT_D3C_PC34,  0, 0, 79,  65, 64, 8},  {0}, 0},
    {3,  1, {DM1_GFX_FLOOR_PIT_D3L_PC34,  0, 0, 142, 65, 78, 8},  {0}, 0},
    {2, -1, {DM1_GFX_FLOOR_PIT_D2L_PC34,  1, 0, 0,   76, 71, 13},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D2L_PC34, 1, 0, 0,   76, 71, 12}, 1},
    {2,  0, {DM1_GFX_FLOOR_PIT_D2C_PC34,  0, 0, 66,  77, 92, 12},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D2C_PC34, 0, 0, 66,  77, 92, 12}, 1},
    {2,  1, {DM1_GFX_FLOOR_PIT_D2L_PC34,  0, 0, 153, 76, 71, 13},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D2L_PC34, 0, 0, 153, 76, 71, 12}, 1},
    {1, -1, {DM1_GFX_FLOOR_PIT_D1L_PC34,  3, 0, 0,   94, 54, 24},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D1L_PC34, 3, 0, 0,   94, 49, 24}, 1},
    {1,  0, {DM1_GFX_FLOOR_PIT_D1C_PC34,  0, 0, 43,  94, 139, 24},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D1C_PC34, 0, 0, 41,  94, 144, 24}, 1},
    {1,  1, {DM1_GFX_FLOOR_PIT_D1L_PC34,  0, 0, 169, 94, 55, 24},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D1L_PC34, 0, 0, 174, 94, 50, 24}, 1},
    {0, -1, {DM1_GFX_FLOOR_PIT_D0L_PC34,  4, 0, 0,   126, 20, 10},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D0L_PC34, 4, 0, 0,   126, 14, 10}, 1},
    {0,  0, {DM1_GFX_FLOOR_PIT_D0C_PC34,  0, 0, 27,  127, 170, 9},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D0C_PC34, 0, 0, 25,  127, 174, 9}, 1},
    {0,  1, {DM1_GFX_FLOOR_PIT_D0L_PC34,  0, 0, 200, 126, 24, 10},
             {DM1_GFX_FLOOR_PIT_INVISIBLE_D0L_PC34, 0, 0, 206, 126, 18, 10}, 1}
};

int dm1_v1_floor_pit_plan_count_pc34(void)
{
    return (int)(sizeof(s_floorPitPlans) / sizeof(s_floorPitPlans[0]));
}

int dm1_v1_floor_pit_render_plan_at_pc34(
    int planIndex,
    DM1_FloorPitRenderPlanPc34* outPlan)
{
    if (planIndex < 0 || planIndex >= dm1_v1_floor_pit_plan_count_pc34() ||
        !outPlan) {
        return 0;
    }
    *outPlan = s_floorPitPlans[planIndex];
    return 1;
}

int dm1_v1_floor_pit_render_plan_pc34(
    int relForward,
    int relSide,
    DM1_FloorPitRenderPlanPc34* outPlan)
{
    int i;
    for (i = 0; i < dm1_v1_floor_pit_plan_count_pc34(); ++i) {
        if (s_floorPitPlans[i].relForward == relForward &&
            s_floorPitPlans[i].relSide == relSide) {
            if (outPlan) {
                *outPlan = s_floorPitPlans[i];
            }
            return outPlan != 0;
        }
    }
    return 0;
}

int dm1_v1_floor_pit_square_draws_pc34(int square)
{
    return (square & DM1_V1_FLOOR_PIT_OPEN_MASK_PC34) != 0;
}

int dm1_v1_floor_pit_square_uses_invisible_pc34(int square)
{
    return dm1_v1_floor_pit_square_draws_pc34(square) &&
           ((square & DM1_V1_FLOOR_PIT_INVISIBLE_MASK_PC34) != 0);
}

int dm1_v1_floor_pit_original_material_receipt_pc34(
    int relForward,
    int relSide,
    int square,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    DM1_V1_FloorFeatureMaterialReceiptPc34* outReceipt)
{
    DM1_FloorPitRenderPlanPc34 plan;
    const DM1_FloorPitBlitPc34* blit;
    DM1_V1_FloorFeatureMaterialReceiptPc34 receipt;
    static const unsigned char kNativePalette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_floor_pit_square_draws_pc34(square) ||
        !dm1_v1_floor_pit_render_plan_pc34(relForward, relSide, &plan)) {
        return 0;
    }
    blit = dm1_v1_floor_pit_square_uses_invisible_pc34(square) &&
        plan.hasInvisibleBlit ? &plan.invisibleBlit : &plan.visibleBlit;
    if (!DM1_V1_FloorFeatureFindSourceMaterialPc34(
            materials, materialCount, blit->graphicIndex, blit->srcX,
            blit->srcY, blit->width, blit->height, &receipt.sourcePixelsFNV1a)) {
        return 0;
    }
    receipt.valid = 1;
    receipt.graphicIndex = blit->graphicIndex;
    receipt.srcX = blit->srcX;
    receipt.srcY = blit->srcY;
    receipt.dstX = blit->dstX;
    receipt.dstY = blit->dstY;
    receipt.width = blit->width;
    receipt.height = blit->height;
    receipt.paletteRoute = DM1_V1_FLOOR_FEATURE_PALETTE_NATIVE_PC34;
    memcpy(receipt.paletteMap, kNativePalette, sizeof(receipt.paletteMap));
    receipt.paletteFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        receipt.paletteMap, (int)sizeof(receipt.paletteMap));
    if (receipt.paletteFNV1a == 0u) return 0;
    *outReceipt = receipt;
    return 1;
}
