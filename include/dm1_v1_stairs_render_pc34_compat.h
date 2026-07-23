#ifndef FIRESTAFF_DM1_V1_STAIRS_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STAIRS_RENDER_PC34_COMPAT_H

#include "dm1_v1_floor_feature_material_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_STAIRS_UP_MASK_PC34 = 0x04,
    DM1_V1_STAIRS_NORTH_SOUTH_MASK_PC34 = 0x08
};

typedef struct DM1_StairsBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_StairsBlitPc34;

typedef struct DM1_StairsRenderPlanPc34 {
    int relForward;
    int relSide;
    int frontOnly;
    int sideOnly;
    DM1_StairsBlitPc34 upBlit;
    DM1_StairsBlitPc34 downBlit;
} DM1_StairsRenderPlanPc34;

int dm1_v1_stairs_render_plan_count_pc34(void);

int dm1_v1_stairs_render_plan_at_pc34(
    int planIndex,
    DM1_StairsRenderPlanPc34* outPlan);

int dm1_v1_stairs_render_plan_pc34(
    int relForward,
    int relSide,
    DM1_StairsRenderPlanPc34* outPlan);

int dm1_v1_stairs_render_plan_for_facing_pc34(
    int relForward,
    int relSide,
    int frontFacing,
    DM1_StairsRenderPlanPc34* outPlan);

int dm1_v1_stairs_front_facing_pc34(int square, int partyDirection);

int dm1_v1_stairs_render_plan_for_square_pc34(
    int relForward,
    int relSide,
    int square,
    int partyDirection,
    DM1_StairsRenderPlanPc34* outPlan);

int dm1_v1_stairs_square_is_up_pc34(int square);

int dm1_v1_stairs_original_material_receipt_pc34(
    int relForward,
    int relSide,
    int square,
    int partyDirection,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    DM1_V1_FloorFeatureMaterialReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_STAIRS_RENDER_PC34_COMPAT_H */
