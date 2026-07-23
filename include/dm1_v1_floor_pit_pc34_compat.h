#ifndef FIRESTAFF_DM1_V1_FLOOR_PIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FLOOR_PIT_PC34_COMPAT_H

#include "dm1_v1_floor_feature_material_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_FLOOR_PIT_OPEN_MASK_PC34 = 0x08,
    DM1_V1_FLOOR_PIT_INVISIBLE_MASK_PC34 = 0x04
};

typedef struct DM1_FloorPitBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_FloorPitBlitPc34;

typedef struct DM1_FloorPitRenderPlanPc34 {
    int relForward;
    int relSide;
    DM1_FloorPitBlitPc34 visibleBlit;
    DM1_FloorPitBlitPc34 invisibleBlit;
    int hasInvisibleBlit;
} DM1_FloorPitRenderPlanPc34;

int dm1_v1_floor_pit_plan_count_pc34(void);

int dm1_v1_floor_pit_render_plan_at_pc34(
    int planIndex,
    DM1_FloorPitRenderPlanPc34* outPlan);

int dm1_v1_floor_pit_render_plan_pc34(
    int relForward,
    int relSide,
    DM1_FloorPitRenderPlanPc34* outPlan);

int dm1_v1_floor_pit_square_draws_pc34(int square);

int dm1_v1_floor_pit_square_uses_invisible_pc34(int square);

int dm1_v1_floor_pit_original_material_receipt_pc34(
    int relForward,
    int relSide,
    int square,
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    DM1_V1_FloorFeatureMaterialReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FLOOR_PIT_PC34_COMPAT_H */
