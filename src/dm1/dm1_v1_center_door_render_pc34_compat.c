#include "dm1_v1_center_door_render_pc34_compat.h"

#include "firestaff/dm1/v1/G0180_pc34_compat.h"
#include "firestaff/dm1/v1/G0183_pc34_compat.h"
#include "firestaff/dm1/v1/G0186_pc34_compat.h"

#include <string.h>

enum {
    DM1_GFX_DOOR_SIDE_D1_PC34 = 87,
    DM1_GFX_DOOR_SIDE_D2_PC34 = 88,
    DM1_GFX_DOOR_SIDE_D3_PC34 = 89,
    DM1_GFX_DOOR_FRAME_TOP_D1_PC34 = 91,
    DM1_GFX_DOOR_FRAME_TOP_D2_PC34 = 92,
    DM1_GFX_DOOR_SET0_D3_PC34 = 246,
    DM1_GFX_DOOR_SET0_D2_PC34 = 247,
    DM1_GFX_DOOR_SET0_D1_PC34 = 248
};

/* ReDMCSB: DUNVIEW.C F0111 lines ~4218-4337 owns center-door
 * panel composition. G0186/G0183/G0180 are the PC34 DOOR_FRAMES rows
 * for D1C/D2C/D3C; M11 only consumes these render plans. */
static const DM1_CenterDoorRenderPlanPc34 s_centerDoorPlans[] = {
    {
        0, 1, 0,
        {DM1_GFX_DOOR_FRAME_TOP_D1_PC34, 0, 0, 61,  12, 102, 4},
        {DM1_GFX_DOOR_SIDE_D1_PC34,      0, 0, 44,  13, 25,  94},
        {DM1_GFX_DOOR_SIDE_D1_PC34,      0, 0, 155, 13, 25,  94},
        3,
        {DM1_GFX_DOOR_SET0_D1_PC34,      0, 0, 64,  16, 96,  86}
    },
    {
        1, 2, 0,
        {DM1_GFX_DOOR_FRAME_TOP_D2_PC34, 0, 0, 77,  21, 70, 3},
        {DM1_GFX_DOOR_SIDE_D2_PC34,      0, 0, 65,  21, 18, 65},
        {DM1_GFX_DOOR_SIDE_D2_PC34,      0, 0, 141, 21, 18, 65},
        3,
        {DM1_GFX_DOOR_SET0_D2_PC34,      0, 0, 80,  24, 64, 59}
    },
    {
        2, 3, 0,
        {DM1_GFX_DOOR_SIDE_D3_PC34,      0, 0, 82,  27, 10, 42},
        {DM1_GFX_DOOR_SIDE_D3_PC34,      0, 0, 132, 27, 10, 42},
        {0},
        2,
        {DM1_GFX_DOOR_SET0_D3_PC34,      0, 0, 90,  30, 44, 38}
    }
};

static int center_door_frame_get_pc34(int depthIndex,
                                      int frameIndex,
                                      int valueIndex)
{
    switch (depthIndex) {
        case 0:
            return dm1_v1_g0186_get_pc34(frameIndex, valueIndex);
        case 1:
            return dm1_v1_g0183_get_pc34(frameIndex, valueIndex);
        case 2:
            return dm1_v1_g0180_get_pc34(frameIndex, valueIndex);
        default:
            return -1;
    }
}

static int center_door_graphic_for_depth_pc34(int depthIndex)
{
    if (depthIndex == 0) {
        return DM1_GFX_DOOR_SET0_D1_PC34;
    }
    if (depthIndex == 1) {
        return DM1_GFX_DOOR_SET0_D2_PC34;
    }
    if (depthIndex == 2) {
        return DM1_GFX_DOOR_SET0_D3_PC34;
    }
    return -1;
}

static int center_door_frame_blit_pc34(int depthIndex,
                                       int frameIndex,
                                       DM1_CenterDoorBlitPc34* outBlit)
{
    int x1;
    int x2;
    int y1;
    int y2;
    int srcX;
    int srcY;
    int graphic;
    if (!outBlit) {
        return 0;
    }
    graphic = center_door_graphic_for_depth_pc34(depthIndex);
    x1 = center_door_frame_get_pc34(depthIndex, frameIndex, 0);
    x2 = center_door_frame_get_pc34(depthIndex, frameIndex, 1);
    y1 = center_door_frame_get_pc34(depthIndex, frameIndex, 2);
    y2 = center_door_frame_get_pc34(depthIndex, frameIndex, 3);
    srcX = center_door_frame_get_pc34(depthIndex, frameIndex, 6);
    srcY = center_door_frame_get_pc34(depthIndex, frameIndex, 7);
    if (graphic < 0 || x1 < 0 || x2 < x1 || y1 < 0 || y2 < y1 ||
        srcX < 0 || srcY < 0) {
        return 0;
    }
    outBlit->graphicIndex = graphic;
    outBlit->srcX = srcX;
    outBlit->srcY = srcY;
    outBlit->dstX = x1;
    outBlit->dstY = y1;
    outBlit->width = x2 - x1 + 1;
    outBlit->height = y2 - y1 + 1;
    return 1;
}

int dm1_v1_center_door_render_plan_count_pc34(void)
{
    return (int)(sizeof(s_centerDoorPlans) / sizeof(s_centerDoorPlans[0]));
}

int dm1_v1_center_door_render_plan_at_pc34(
    int planIndex,
    DM1_CenterDoorRenderPlanPc34* outPlan)
{
    if (!outPlan ||
        planIndex < 0 ||
        planIndex >= dm1_v1_center_door_render_plan_count_pc34()) {
        return 0;
    }
    *outPlan = s_centerDoorPlans[planIndex];
    return 1;
}

int dm1_v1_center_door_render_plan_for_depth_pc34(
    int depthIndex,
    DM1_CenterDoorRenderPlanPc34* outPlan)
{
    if (!outPlan || depthIndex < 0 ||
        depthIndex >= dm1_v1_center_door_render_plan_count_pc34()) {
        return 0;
    }
    *outPlan = s_centerDoorPlans[depthIndex];
    return 1;
}

int dm1_v1_center_door_panel_blits_for_cell_pc34(
    int depthIndex,
    int doorState,
    int doorVertical,
    DM1_CenterDoorBlitPc34 outBlits[2])
{
    DM1_CenterDoorRenderPlanPc34 plan;
    if (!outBlits || doorState == 0 ||
        !dm1_v1_center_door_render_plan_for_depth_pc34(depthIndex, &plan)) {
        return 0;
    }
    if (!doorVertical && doorState >= 1 && doorState <= 3) {
        int stateIndex = doorState - 1;
        int leftFrame = 4 + stateIndex;
        int rightFrame = 7 + stateIndex;
        if (!center_door_frame_blit_pc34(depthIndex, leftFrame, &outBlits[0])) {
            return 0;
        }
        if (!center_door_frame_blit_pc34(depthIndex, rightFrame, &outBlits[1])) {
            return 0;
        }
        return 2;
    }
    if (doorState >= 1 && doorState <= 3) {
        static const int kDoorOpenSrcY[3][4] = {
            {0, 65, 43, 21},
            {0, 44, 29, 14},
            {0, 27, 17, 7}
        };
        static const int kDoorOpenHeight[3][4] = {
            {0, 23, 45, 67},
            {0, 17, 32, 47},
            {0, 11, 21, 31}
        };
        outBlits[0] = plan.closedPanel;
        outBlits[0].srcY = kDoorOpenSrcY[depthIndex][doorState];
        if (depthIndex == 0) {
            /* ReDMCSB: DUNVIEW.C F0111 lines 4289-4334 applies the D1C
             * thieves-eye mask to the temporary D1C door bitmap before the
             * partly-open F0102/F0791 clip.  The resulting D1C vertical panel
             * blit is one pixel above the closed-panel anchor. */
            outBlits[0].dstY -= 1;
        }
        outBlits[0].height = kDoorOpenHeight[depthIndex][doorState];
        return 1;
    }
    outBlits[0] = plan.closedPanel;
    return 1;
}

int dm1_v1_center_door_host_material_receipt_pc34(
    int depthIndex,
    int doorState,
    int doorVertical,
    int panelGraphicIndex,
    DM1_CenterDoorHostMaterialReceiptPc34* outReceipt)
{
    DM1_CenterDoorRenderPlanPc34 plan;
    DM1_CenterDoorBlitPc34 panels[2];
    int panelCount = 0;
    int i;

    if (!outReceipt || doorState < 0 || doorState > 7 ||
        !dm1_v1_center_door_render_plan_for_depth_pc34(depthIndex, &plan)) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->depthIndex = depthIndex;
    outReceipt->doorState = doorState;
    outReceipt->doorVertical = doorVertical ? 1 : 0;
    outReceipt->frameCount = plan.frameCount;
    for (i = 0; i < plan.frameCount; ++i) {
        outReceipt->blits[outReceipt->blitCount++] =
            i == 0 ? plan.frameA : (i == 1 ? plan.frameB : plan.frameC);
    }
    /* ReDMCSB DUNVIEW.C F0111:4218-4337 draws door frames for every state,
     * then omits the panel only for C0_DOOR_STATE_OPEN. The map-resolved
     * F0095 bitmap may replace the plan's set-0 panel, never its geometry. */
    if (doorState != 0) {
        panelCount = dm1_v1_center_door_panel_blits_for_cell_pc34(
            depthIndex, doorState, doorVertical, panels);
        if (panelCount < 1 || panelCount > 2 ||
            outReceipt->blitCount + panelCount >
                DM1_CENTER_DOOR_HOST_MATERIAL_MAX_BLITS) {
            return 0;
        }
        for (i = 0; i < panelCount; ++i) {
            if (panelGraphicIndex >= 0) {
                panels[i].graphicIndex = panelGraphicIndex;
            }
            outReceipt->blits[outReceipt->blitCount++] = panels[i];
        }
        outReceipt->panelVisible = 1;
    }
    outReceipt->valid = 1;
    return 1;
}

int dm1_v1_door_panel_graphic_for_set_depth_pc34(
    int doorSet,
    int depthIndex)
{
    int depthOffset;
    if (doorSet < 0 || depthIndex < 0 || depthIndex > 2) {
        return -1;
    }
    /* ReDMCSB F0095_DUNGEONVIEW_LoadDoorSet:
     * D3 = M633 + doorSet * 3 + 0
     * D2 = M633 + doorSet * 3 + 1
     * D1 = M633 + doorSet * 3 + 2. */
    depthOffset = 2 - depthIndex;
    return DM1_GFX_DOOR_SET0_D3_PC34 + doorSet * 3 + depthOffset;
}

int dm1_v1_door_panel_graphic_for_map_depth_pc34(
    int doorType,
    int mapDoorSet0,
    int mapDoorSet1,
    int depthIndex)
{
    int doorSet = (doorType & 1) ? mapDoorSet1 : mapDoorSet0;
    return dm1_v1_door_panel_graphic_for_set_depth_pc34(doorSet, depthIndex);
}

int dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int doorType,
    int depthIndex)
{
    if (!dungeon || !dungeon->maps || mapIndex < 0 ||
        mapIndex >= dungeon->header.mapCount) {
        return -1;
    }
    return dm1_v1_door_panel_graphic_for_map_depth_pc34(
        doorType,
        (int)dungeon->maps[mapIndex].doorSet0,
        (int)dungeon->maps[mapIndex].doorSet1,
        depthIndex);
}
