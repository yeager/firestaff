#include "dm1_v1_side_door_render_pc34_compat.h"

#include "firestaff/dm1/v1/G0179_pc34_compat.h"
#include "firestaff/dm1/v1/G0181_pc34_compat.h"
#include "firestaff/dm1/v1/G0182_pc34_compat.h"
#include "firestaff/dm1/v1/G0184_pc34_compat.h"
#include "firestaff/dm1/v1/G0185_pc34_compat.h"
#include "firestaff/dm1/v1/G0187_pc34_compat.h"

enum {
    DM1_GFX_DOOR_FRAME_D3W_PC34 = 90,
    DM1_GFX_DOOR_FRAME_TOP_D2_PC34 = 92,
    DM1_GFX_DOOR_FRAME_TOP_D1_PC34 = 91,
    DM1_GFX_DOOR_SET0_D3_PC34 = 246,
    DM1_GFX_DOOR_SET0_D2_PC34 = 247,
    DM1_GFX_DOOR_SET0_D1_PC34 = 248
};

/* ReDMCSB: DUNVIEW.C F0111 lines ~4218-4337 owns door panel
 * composition. DEFS.H/G0179..G0187 bind D3/D2/D1 side-door frame
 * rectangles; D3L2/D3R2 use clipped door panel slices without side frames. */
static const DM1_SideDoorRenderPlanPc34 s_sideDoorPlans[] = {
    {3, -2, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 35, 0, 0,   28, 9,  38}, {0}, {0}, 0},
    {3,  2, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 0,  0, 210, 28, 14, 38}, {0}, {0}, 0},
    {3, -1, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 1,  0, 30,  29, 43, 38},
               {DM1_GFX_DOOR_FRAME_D3W_PC34, 0, 0, 16, 27, 16, 43},
               {DM1_GFX_DOOR_FRAME_D3W_PC34, 0, 0, 73, 27, 16, 43}, 2},
    {3,  1, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 0,  0, 151, 29, 43, 38},
               {DM1_GFX_DOOR_FRAME_D3W_PC34, 0, 0, 147, 27, 16, 43},
               {DM1_GFX_DOOR_FRAME_D3W_PC34, 0, 0, 192, 26, 16, 43}, 2},
    {2, -1, 1, {DM1_GFX_DOOR_SET0_D2_PC34, 4, 0, 0,   24, 60, 59},
               {DM1_GFX_DOOR_FRAME_TOP_D2_PC34, 0, 0, 6, 22, 70, 3}, {0}, 1},
    {2,  1, 1, {DM1_GFX_DOOR_SET0_D2_PC34, 0, 0, 164, 23, 60, 59},
               {DM1_GFX_DOOR_FRAME_TOP_D2_PC34, 0, 0, 160, 22, 64, 3}, {0}, 1},
    {1, -1, 0, {DM1_GFX_DOOR_SET0_D1_PC34, 64, 0, 0,   18, 32, 86},
               {DM1_GFX_DOOR_FRAME_TOP_D1_PC34, 0, 0, 0, 14, 102, 4}, {0}, 1},
    {1,  1, 0, {DM1_GFX_DOOR_SET0_D1_PC34, 0,  0, 192, 18, 32, 86},
               {DM1_GFX_DOOR_FRAME_TOP_D1_PC34, 0, 0, 122, 14, 102, 4}, {0}, 1}
};

static int side_door_frame_get_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int frameIndex,
    int valueIndex)
{
    if (!plan) {
        return -1;
    }
    if (plan->relForward == 3 && plan->relSide == -1) {
        return dm1_v1_g0179_get_pc34(frameIndex, valueIndex);
    }
    if (plan->relForward == 3 && plan->relSide == 1) {
        return dm1_v1_g0181_get_pc34(frameIndex, valueIndex);
    }
    if (plan->relForward == 2 && plan->relSide == -1) {
        return dm1_v1_g0182_get_pc34(frameIndex, valueIndex);
    }
    if (plan->relForward == 2 && plan->relSide == 1) {
        return dm1_v1_g0184_get_pc34(frameIndex, valueIndex);
    }
    if (plan->relForward == 1 && plan->relSide == -1) {
        return dm1_v1_g0185_get_pc34(frameIndex, valueIndex);
    }
    if (plan->relForward == 1 && plan->relSide == 1) {
        return dm1_v1_g0187_get_pc34(frameIndex, valueIndex);
    }
    return -1;
}

static int side_door_frame_blit_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int frameIndex,
    DM1_SideDoorBlitPc34* outBlit)
{
    int x1;
    int x2;
    int y1;
    int y2;
    int srcX;
    int srcY;
    if (!plan || !outBlit) {
        return 0;
    }
    x1 = side_door_frame_get_pc34(plan, frameIndex, 0);
    x2 = side_door_frame_get_pc34(plan, frameIndex, 1);
    y1 = side_door_frame_get_pc34(plan, frameIndex, 2);
    y2 = side_door_frame_get_pc34(plan, frameIndex, 3);
    srcX = side_door_frame_get_pc34(plan, frameIndex, 6);
    srcY = side_door_frame_get_pc34(plan, frameIndex, 7);
    if (x1 < 0 || x2 < x1 || y1 < 0 || y2 < y1 ||
        srcX < 0 || srcY < 0) {
        return 0;
    }
    if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0 &&
        srcX == 0 && srcY == 0) {
        return 0;
    }
    *outBlit = plan->panel;
    outBlit->srcX = srcX;
    outBlit->srcY = srcY;
    outBlit->dstX = x1;
    outBlit->dstY = y1;
    outBlit->width = x2 - x1 + 1;
    outBlit->height = y2 - y1 + 1;
    return 1;
}

int dm1_v1_side_door_render_plan_count_pc34(void)
{
    return (int)(sizeof(s_sideDoorPlans) / sizeof(s_sideDoorPlans[0]));
}

int dm1_v1_side_door_render_plan_at_pc34(
    int planIndex,
    DM1_SideDoorRenderPlanPc34* outPlan)
{
    if (planIndex < 0 ||
        planIndex >= dm1_v1_side_door_render_plan_count_pc34() ||
        !outPlan) {
        return 0;
    }
    *outPlan = s_sideDoorPlans[planIndex];
    return 1;
}

int dm1_v1_side_door_render_plan_for_rel_pc34(
    int relForward,
    int relSide,
    DM1_SideDoorRenderPlanPc34* outPlan)
{
    int i;
    if (!outPlan) {
        return 0;
    }
    for (i = 0; i < dm1_v1_side_door_render_plan_count_pc34(); ++i) {
        if (s_sideDoorPlans[i].relForward == relForward &&
            s_sideDoorPlans[i].relSide == relSide) {
            *outPlan = s_sideDoorPlans[i];
            return 1;
        }
    }
    return 0;
}

int dm1_v1_side_door_panel_blits_for_cell_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2])
{
    int frameIndex;
    int count = 0;
    if (!plan || !outBlits || doorState == 0) {
        return 0;
    }
    if (doorState >= 1 && doorState <= 3) {
        int stateIndex = doorState - 1;
        if (doorVertical) {
            frameIndex = 1 + stateIndex;
            return side_door_frame_blit_pc34(plan, frameIndex, &outBlits[0]) ? 1 : 0;
        }
        frameIndex = 4 + stateIndex;
        if (side_door_frame_blit_pc34(plan, frameIndex, &outBlits[count])) {
            ++count;
        }
        frameIndex = 7 + stateIndex;
        if (side_door_frame_blit_pc34(plan, frameIndex, &outBlits[count])) {
            ++count;
        }
        return count;
    }
    return side_door_frame_blit_pc34(plan, 0, &outBlits[0]) ? 1 : 0;
}

int dm1_v1_side_door_panel_blits_for_draw_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2])
{
    int count = dm1_v1_side_door_panel_blits_for_cell_pc34(
        plan,
        doorState,
        doorVertical,
        outBlits);
    if (count > 0) {
        return count;
    }
    if (!plan || !outBlits || doorState == 0) {
        return 0;
    }
    outBlits[0] = plan->panel;
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
        outBlits[0].srcY = kDoorOpenSrcY[plan->depthIndex][doorState];
        outBlits[0].height = kDoorOpenHeight[plan->depthIndex][doorState];
        if (plan->depthIndex == 2) {
            outBlits[0].dstY = (plan->relSide == -1 || plan->relSide == 1) ? 29 : 28;
            if (plan->relSide == -1) {
                outBlits[0].srcX = 0;
                outBlits[0].dstX = 30;
                outBlits[0].width = 44;
            } else if (plan->relSide == 1) {
                outBlits[0].srcX = 0;
                outBlits[0].dstX = 150;
                outBlits[0].width = 44;
            }
        }
    }
    return 1;
}
