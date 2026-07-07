#include "dm1_v1_floor_pit_pc34_compat.h"

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
                        int visibleGraphic,
                        int visibleDstX,
                        int visibleDstY,
                        int visibleWidth,
                        int visibleHeight,
                        int hasInvisible,
                        int invisibleGraphic)
{
    DM1_FloorPitRenderPlanPc34 plan;
    int ok = dm1_v1_floor_pit_render_plan_pc34(relForward, relSide, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("plan.relForward", plan.relForward, relForward);
    expect_int("plan.relSide", plan.relSide, relSide);
    expect_int("plan.visible.graphic", plan.visibleBlit.graphicIndex, visibleGraphic);
    expect_int("plan.visible.dstX", plan.visibleBlit.dstX, visibleDstX);
    expect_int("plan.visible.dstY", plan.visibleBlit.dstY, visibleDstY);
    expect_int("plan.visible.width", plan.visibleBlit.width, visibleWidth);
    expect_int("plan.visible.height", plan.visibleBlit.height, visibleHeight);
    expect_int("plan.hasInvisible", plan.hasInvisibleBlit, hasInvisible);
    expect_int("plan.invisible.graphic",
               hasInvisible ? plan.invisibleBlit.graphicIndex : 0,
               invisibleGraphic);
}

int main(void)
{
    DM1_FloorPitRenderPlanPc34 plan;

    expect_int("plan.count", dm1_v1_floor_pit_plan_count_pc34(), 14);

    expect_plan("plan.d3l2", 3, -2, 49, 0,   66, 22, 10, 0, 0);
    expect_plan("plan.d3c",  3,  0, 51, 79,  65, 64, 8,  0, 0);
    expect_plan("plan.d2c",  2,  0, 53, 66,  77, 92, 12, 1, 59);
    expect_plan("plan.d1r",  1,  1, 54, 169, 94, 55, 24, 1, 60);
    expect_plan("plan.d0c",  0,  0, 57, 27,  127, 170, 9, 1, 63);

    expect_int("closed.draws", dm1_v1_floor_pit_square_draws_pc34(0x40), 0);
    expect_int("open.draws",
               dm1_v1_floor_pit_square_draws_pc34(0x40 | 0x08), 1);
    expect_int("open.visible",
               dm1_v1_floor_pit_square_uses_invisible_pc34(0x40 | 0x08), 0);
    expect_int("open.invisible",
               dm1_v1_floor_pit_square_uses_invisible_pc34(0x40 | 0x08 | 0x04), 1);

    expect_int("bad.rel",
               dm1_v1_floor_pit_render_plan_pc34(4, 0, &plan), 0);
    expect_int("bad.null",
               dm1_v1_floor_pit_render_plan_pc34(0, 0, 0), 0);
    expect_int("at.valid",
               dm1_v1_floor_pit_render_plan_at_pc34(13, &plan), 1);
    expect_int("at.bad",
               dm1_v1_floor_pit_render_plan_at_pc34(14, &plan), 0);
    expect_int("at.null",
               dm1_v1_floor_pit_render_plan_at_pc34(0, 0), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
