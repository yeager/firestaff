#include "dm1_v1_stairs_render_pc34_compat.h"

#include <stdio.h>

static int g_passed;
static int g_failed;

static void expect_int(const char *name, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", name, got, want);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, want);
    ++g_passed;
}

static void expect_plan(const char *name,
                        int relForward,
                        int relSide,
                        int frontFacing,
                        int frontOnly,
                        int sideOnly,
                        int upGraphic,
                        int upDstX,
                        int upDstY,
                        int upWidth,
                        int upHeight,
                        int downGraphic,
                        int downDstX,
                        int downDstY,
                        int downWidth,
                        int downHeight)
{
    DM1_StairsRenderPlanPc34 plan;
    int ok = dm1_v1_stairs_render_plan_for_facing_pc34(
        relForward,
        relSide,
        frontFacing,
        &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("plan.relForward", plan.relForward, relForward);
    expect_int("plan.relSide", plan.relSide, relSide);
    expect_int("plan.frontOnly", plan.frontOnly, frontOnly);
    expect_int("plan.sideOnly", plan.sideOnly, sideOnly);
    expect_int("plan.up.graphic", plan.upBlit.graphicIndex, upGraphic);
    expect_int("plan.up.dstX", plan.upBlit.dstX, upDstX);
    expect_int("plan.up.dstY", plan.upBlit.dstY, upDstY);
    expect_int("plan.up.width", plan.upBlit.width, upWidth);
    expect_int("plan.up.height", plan.upBlit.height, upHeight);
    expect_int("plan.down.graphic", plan.downBlit.graphicIndex, downGraphic);
    expect_int("plan.down.dstX", plan.downBlit.dstX, downDstX);
    expect_int("plan.down.dstY", plan.downBlit.dstY, downDstY);
    expect_int("plan.down.width", plan.downBlit.width, downWidth);
    expect_int("plan.down.height", plan.downBlit.height, downHeight);
}

int main(void)
{
    DM1_StairsRenderPlanPc34 plan;

    expect_int("plan.count", dm1_v1_stairs_render_plan_count_pc34(), 19);

    expect_plan("plan.d3l2.front", 3, -2, 1, 1, 0,
                108, 0, 25, 63, 45,
                115, 0, 25, 75, 41);
    expect_plan("plan.d3c.front", 3, 0, 1, 1, 0,
                109, 78, 25, 68, 46,
                116, 75, 25, 74, 49);
    expect_plan("plan.d2c.front", 2, 0, 1, 1, 0,
                111, 62, 20, 100, 63,
                118, 63, 24, 98, 61);
    expect_plan("plan.d1r.front", 1, 1, 1, 1, 0,
                112, 192, 9, 32, 100,
                119, 192, 18, 32, 91);
    expect_plan("plan.d1l.side", 1, -1, 0, 0, 1,
                123, 32, 58, 20, 43,
                124, 32, 62, 20, 39);
    expect_plan("plan.d0r.side", 0, 1, 0, 0, 1,
                125, 208, 73, 16, 13,
                125, 208, 73, 16, 13);

    expect_int("square.down", dm1_v1_stairs_square_is_up_pc34(0x60), 0);
    expect_int("square.up", dm1_v1_stairs_square_is_up_pc34(0x60 | 0x04), 1);
    expect_int("facing.ns.north",
               dm1_v1_stairs_front_facing_pc34(0x60 | 0x08, 0), 1);
    expect_int("facing.ns.east",
               dm1_v1_stairs_front_facing_pc34(0x60 | 0x08, 1), 0);
    expect_int("facing.ns.south",
               dm1_v1_stairs_front_facing_pc34(0x60 | 0x08, 2), 1);
    expect_int("facing.ew.east",
               dm1_v1_stairs_front_facing_pc34(0x60, 1), 1);
    expect_int("facing.ew.west",
               dm1_v1_stairs_front_facing_pc34(0x60, 3), 1);
    expect_int("facing.direction_wrap",
               dm1_v1_stairs_front_facing_pc34(0x60 | 0x08, 4), 1);
    expect_int("plan.square.front",
               dm1_v1_stairs_render_plan_for_square_pc34(
                   1, 0, 0x60 | 0x08, 0, &plan), 1);
    expect_int("plan.square.frontOnly", plan.frontOnly, 1);
    expect_int("plan.square.side",
               dm1_v1_stairs_render_plan_for_square_pc34(
                   1, -1, 0x60, 0, &plan), 1);
    expect_int("plan.square.sideOnly", plan.sideOnly, 1);
    expect_int("bad.rel", dm1_v1_stairs_render_plan_pc34(4, 0, &plan), 0);
    expect_int("bad.null", dm1_v1_stairs_render_plan_pc34(0, 1, 0), 0);
    expect_int("bad.facing.rel", dm1_v1_stairs_render_plan_for_facing_pc34(3, -2, 0, &plan), 0);
    expect_int("bad.facing.null", dm1_v1_stairs_render_plan_for_facing_pc34(0, 1, 0, 0), 0);
    expect_int("bad.square.null",
               dm1_v1_stairs_render_plan_for_square_pc34(0, 1, 0x60, 0, 0), 0);
    expect_int("at.valid", dm1_v1_stairs_render_plan_at_pc34(18, &plan), 1);
    expect_int("at.bad", dm1_v1_stairs_render_plan_at_pc34(19, &plan), 0);
    expect_int("at.null", dm1_v1_stairs_render_plan_at_pc34(0, 0), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
