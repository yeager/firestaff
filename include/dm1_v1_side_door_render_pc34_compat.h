#ifndef FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H

#include "dm1_v1_door_source_material_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_SideDoorBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_SideDoorBlitPc34;

typedef struct DM1_SideDoorRenderPlanPc34 {
    int relForward;
    int relSide;
    int depthIndex;
    DM1_SideDoorBlitPc34 panel;
    DM1_SideDoorBlitPc34 frameA;
    DM1_SideDoorBlitPc34 frameB;
    int frameCount;
} DM1_SideDoorRenderPlanPc34;

enum {
    DM1_SIDE_DOOR_HOST_MATERIAL_MAX_BLITS = 4
};

typedef struct DM1_SideDoorOriginalMaterialReceiptPc34 {
    int valid;
    int relForward;
    int relSide;
    int depthIndex;
    int doorState;
    int panelVisible;
    int frameCount;
    int blitCount;
    DM1_SideDoorBlitPc34 blits[DM1_SIDE_DOOR_HOST_MATERIAL_MAX_BLITS];
    uint32_t sourcePixelsFNV1a[DM1_SIDE_DOOR_HOST_MATERIAL_MAX_BLITS];
} DM1_SideDoorOriginalMaterialReceiptPc34;

int dm1_v1_side_door_render_plan_count_pc34(void);

int dm1_v1_side_door_render_plan_at_pc34(
    int planIndex,
    DM1_SideDoorRenderPlanPc34* outPlan);

int dm1_v1_side_door_render_plan_for_rel_pc34(
    int relForward,
    int relSide,
    DM1_SideDoorRenderPlanPc34* outPlan);

int dm1_v1_side_door_panel_blits_for_cell_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2]);

int dm1_v1_side_door_panel_blits_for_draw_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2]);
int dm1_v1_side_door_original_material_receipt_pc34(
    int relForward,
    int relSide,
    int doorState,
    int doorVertical,
    int panelGraphic,
    const DM1_V1_DoorSourceMaterialPc34* materials,
    int materialCount,
    DM1_SideDoorOriginalMaterialReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H */
