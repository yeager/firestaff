#include "dm1_v1_stairs_render_pc34_compat.h"

/* ReDMCSB PC34: DEFS.H lines 2374 and 4139-4172 bind
 * M645_GRAPHIC_FIRST_STAIRS=108 and the D3L2..D0R stairs zones.
 * DUNGEON.C F0172 line 2695 stores MASK0x0004_STAIRS_UP in
 * M555_STAIRS_UP for DUNVIEW.C F0104/F0105 dispatch. */
enum {
    DM1_GFX_STAIRS_UP_FRONT_D3L_PC34 = 108,
    DM1_GFX_STAIRS_UP_FRONT_D3C_PC34 = 109,
    DM1_GFX_STAIRS_UP_FRONT_D2L_PC34 = 110,
    DM1_GFX_STAIRS_UP_FRONT_D2C_PC34 = 111,
    DM1_GFX_STAIRS_UP_FRONT_D1L_PC34 = 112,
    DM1_GFX_STAIRS_UP_FRONT_D1C_PC34 = 113,
    DM1_GFX_STAIRS_UP_FRONT_D0C_L_PC34 = 114,
    DM1_GFX_STAIRS_DOWN_FRONT_D3L_PC34 = 115,
    DM1_GFX_STAIRS_DOWN_FRONT_D3C_PC34 = 116,
    DM1_GFX_STAIRS_DOWN_FRONT_D2L_PC34 = 117,
    DM1_GFX_STAIRS_DOWN_FRONT_D2C_PC34 = 118,
    DM1_GFX_STAIRS_DOWN_FRONT_D1L_PC34 = 119,
    DM1_GFX_STAIRS_DOWN_FRONT_D1C_PC34 = 120,
    DM1_GFX_STAIRS_DOWN_FRONT_D0C_L_PC34 = 121,
    DM1_GFX_STAIRS_SIDE_D2L_PC34 = 122,
    DM1_GFX_STAIRS_UP_SIDE_D1L_PC34 = 123,
    DM1_GFX_STAIRS_DOWN_SIDE_D1L_PC34 = 124,
    DM1_GFX_STAIRS_SIDE_D0L_PC34 = 125
};

static const DM1_StairsRenderPlanPc34 s_stairsPlans[] = {
    {3, -2, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D3L_PC34, 0, 0, 0,   25, 63, 45}, {DM1_GFX_STAIRS_DOWN_FRONT_D3L_PC34, 0, 0, 0,   25, 75, 41}},
    {3,  2, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D3L_PC34, 0, 0, 161, 25, 63, 45}, {DM1_GFX_STAIRS_DOWN_FRONT_D3L_PC34, 0, 0, 149, 25, 75, 41}},
    {3, -1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D3L_PC34, 0, 0, 14,  26, 63, 45}, {DM1_GFX_STAIRS_DOWN_FRONT_D3L_PC34, 0, 0, 13,  28, 75, 41}},
    {3,  0, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D3C_PC34, 0, 0, 78,  25, 68, 46}, {DM1_GFX_STAIRS_DOWN_FRONT_D3C_PC34, 0, 0, 75,  25, 74, 49}},
    {3,  1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D3L_PC34, 0, 0, 147, 26, 63, 45}, {DM1_GFX_STAIRS_DOWN_FRONT_D3L_PC34, 0, 0, 133, 28, 75, 41}},
    {2, -1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D2L_PC34, 1, 0, 0,   24, 59, 62}, {DM1_GFX_STAIRS_DOWN_FRONT_D2L_PC34, 0, 0, 0,   20, 61, 62}},
    {2,  0, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D2C_PC34, 0, 0, 62,  20, 100, 63}, {DM1_GFX_STAIRS_DOWN_FRONT_D2C_PC34, 0, 0, 63,  24, 98, 61}},
    {2,  1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D2L_PC34, 0, 0, 165, 20, 59, 62}, {DM1_GFX_STAIRS_DOWN_FRONT_D2L_PC34, 0, 0, 164, 24, 60, 62}},
    {1, -1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D1L_PC34, 0, 0, 0,   9,  32, 100}, {DM1_GFX_STAIRS_DOWN_FRONT_D1L_PC34, 0, 0, 0,   17, 32, 91}},
    {1,  0, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D1C_PC34, 0, 0, 32,  9,  160, 100}, {DM1_GFX_STAIRS_DOWN_FRONT_D1C_PC34, 0, 0, 35,  17, 152, 92}},
    {1,  1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D1L_PC34, 0, 0, 192, 9,  32, 100}, {DM1_GFX_STAIRS_DOWN_FRONT_D1L_PC34, 0, 0, 192, 18, 32, 91}},
    {0, -1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D0C_L_PC34, 0, 0, 0,   58, 30, 44}, {DM1_GFX_STAIRS_DOWN_FRONT_D0C_L_PC34, 0, 0, 0,   76, 30, 60}},
    {0,  1, 1, 0, {DM1_GFX_STAIRS_UP_FRONT_D0C_L_PC34, 0, 0, 194, 58, 30, 44}, {DM1_GFX_STAIRS_DOWN_FRONT_D0C_L_PC34, 0, 0, 194, 76, 30, 60}},
    {2, -1, 0, 1, {DM1_GFX_STAIRS_SIDE_D2L_PC34, 0, 0, 60,  55, 8,  5}, {DM1_GFX_STAIRS_SIDE_D2L_PC34, 0, 0, 60,  55, 8,  5}},
    {2,  1, 0, 1, {DM1_GFX_STAIRS_SIDE_D2L_PC34, 0, 0, 156, 56, 8,  5}, {DM1_GFX_STAIRS_SIDE_D2L_PC34, 0, 0, 156, 56, 8,  5}},
    {1, -1, 0, 1, {DM1_GFX_STAIRS_UP_SIDE_D1L_PC34, 0, 0, 32,  58, 20, 43}, {DM1_GFX_STAIRS_DOWN_SIDE_D1L_PC34, 0, 0, 32,  62, 20, 39}},
    {1,  1, 0, 1, {DM1_GFX_STAIRS_UP_SIDE_D1L_PC34, 0, 0, 172, 57, 20, 43}, {DM1_GFX_STAIRS_DOWN_SIDE_D1L_PC34, 0, 0, 172, 62, 20, 39}},
    {0, -1, 0, 1, {DM1_GFX_STAIRS_SIDE_D0L_PC34, 0, 0, 0,   73, 16, 13}, {DM1_GFX_STAIRS_SIDE_D0L_PC34, 0, 0, 0,   73, 16, 13}},
    {0,  1, 0, 1, {DM1_GFX_STAIRS_SIDE_D0L_PC34, 0, 0, 208, 73, 16, 13}, {DM1_GFX_STAIRS_SIDE_D0L_PC34, 0, 0, 208, 73, 16, 13}}
};

int dm1_v1_stairs_render_plan_count_pc34(void)
{
    return (int)(sizeof(s_stairsPlans) / sizeof(s_stairsPlans[0]));
}

int dm1_v1_stairs_render_plan_at_pc34(
    int planIndex,
    DM1_StairsRenderPlanPc34* outPlan)
{
    if (planIndex < 0 ||
        planIndex >= dm1_v1_stairs_render_plan_count_pc34() ||
        !outPlan) {
        return 0;
    }
    *outPlan = s_stairsPlans[planIndex];
    return 1;
}

int dm1_v1_stairs_render_plan_pc34(
    int relForward,
    int relSide,
    DM1_StairsRenderPlanPc34* outPlan)
{
    int i;
    if (!outPlan) {
        return 0;
    }
    for (i = 0; i < dm1_v1_stairs_render_plan_count_pc34(); ++i) {
        if (s_stairsPlans[i].relForward == relForward &&
            s_stairsPlans[i].relSide == relSide) {
            *outPlan = s_stairsPlans[i];
            return 1;
        }
    }
    return 0;
}

int dm1_v1_stairs_render_plan_for_facing_pc34(
    int relForward,
    int relSide,
    int frontFacing,
    DM1_StairsRenderPlanPc34* outPlan)
{
    int i;
    if (!outPlan) {
        return 0;
    }
    for (i = 0; i < dm1_v1_stairs_render_plan_count_pc34(); ++i) {
        if (s_stairsPlans[i].relForward == relForward &&
            s_stairsPlans[i].relSide == relSide &&
            ((frontFacing && s_stairsPlans[i].frontOnly) ||
             (!frontFacing && s_stairsPlans[i].sideOnly))) {
            *outPlan = s_stairsPlans[i];
            return 1;
        }
    }
    return 0;
}

int dm1_v1_stairs_square_is_up_pc34(int square)
{
    return (square & DM1_V1_STAIRS_UP_MASK_PC34) != 0;
}
