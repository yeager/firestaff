#ifndef FIRESTAFF_DM1_V1_CENTER_DOOR_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CENTER_DOOR_RENDER_PC34_COMPAT_H

#include "dm1_v1_door_source_material_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_CenterDoorBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_CenterDoorBlitPc34;

typedef struct DM1_CenterDoorRenderPlanPc34 {
    int depthIndex;
    int relForward;
    int relSide;
    DM1_CenterDoorBlitPc34 frameA;
    DM1_CenterDoorBlitPc34 frameB;
    DM1_CenterDoorBlitPc34 frameC;
    int frameCount;
    DM1_CenterDoorBlitPc34 closedPanel;
} DM1_CenterDoorRenderPlanPc34;

enum {
    DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS = 5
};

typedef struct DM1_CenterDoorHostMaterialReceiptPc34 {
    int valid;
    int depthIndex;
    int doorState;
    int panelVisible;
    int frameCount;
    int blitCount;
    DM1_CenterDoorBlitPc34 blits[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
} DM1_CenterDoorHostMaterialReceiptPc34;

typedef struct DM1_CenterDoorOriginalMaterialReceiptPc34 {
    DM1_CenterDoorHostMaterialReceiptPc34 plan;
    int sourceMaterialCount;
    uint32_t sourcePixelsFNV1a[DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS];
} DM1_CenterDoorOriginalMaterialReceiptPc34;

int dm1_v1_center_door_render_plan_count_pc34(void);

int dm1_v1_center_door_render_plan_at_pc34(
    int planIndex,
    DM1_CenterDoorRenderPlanPc34* outPlan);

int dm1_v1_center_door_render_plan_for_depth_pc34(
    int depthIndex,
    DM1_CenterDoorRenderPlanPc34* outPlan);

int dm1_v1_center_door_panel_blits_for_cell_pc34(
    int depthIndex,
    int doorState,
    int doorVertical,
    DM1_CenterDoorBlitPc34 outBlits[2]);

int dm1_v1_door_panel_graphic_for_set_depth_pc34(
    int doorSet,
    int depthIndex);
int dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
    const void* dungeon,
    int mapIndex,
    int doorType,
    int depthIndex);
int dm1_v1_center_door_host_material_receipt_pc34(
    int depthIndex,
    int doorState,
    int doorVertical,
    int panelGraphic,
    DM1_CenterDoorHostMaterialReceiptPc34* outReceipt);
int dm1_v1_center_door_original_material_receipt_pc34(
    int depthIndex,
    int doorState,
    int doorVertical,
    int panelGraphic,
    const DM1_V1_DoorSourceMaterialPc34* materials,
    int materialCount,
    DM1_CenterDoorOriginalMaterialReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CENTER_DOOR_RENDER_PC34_COMPAT_H */
