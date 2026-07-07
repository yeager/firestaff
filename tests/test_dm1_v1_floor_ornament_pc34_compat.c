#include "dm1_v1_floor_ornament_pc34_compat.h"

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
                        int globalIndex,
                        int increment,
                        int flip,
                        int graphicIndex,
                        int dstX,
                        int dstY,
                        int width,
                        int height)
{
    DM1_FloorOrnamentRenderPlanPc34 plan;
    int ok = dm1_v1_floor_ornament_render_plan_pc34(
        relForward, relSide, globalIndex, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("plan.relForward", plan.relForward, relForward);
    expect_int("plan.relSide", plan.relSide, relSide);
    expect_int("plan.increment", plan.increment, increment);
    expect_int("plan.flipHorizontal", plan.flipHorizontal, flip);
    expect_int("plan.graphicIndex", plan.blit.graphicIndex, graphicIndex);
    expect_int("plan.srcX", plan.blit.srcX, 0);
    expect_int("plan.srcY", plan.blit.srcY, 0);
    expect_int("plan.dstX", plan.blit.dstX, dstX);
    expect_int("plan.dstY", plan.blit.dstY, dstY);
    expect_int("plan.width", plan.blit.width, width);
    expect_int("plan.height", plan.blit.height, height);
}

static void expect_source_zone(const char *name,
                               int relForward,
                               int relSide,
                               int dstX,
                               int dstY,
                               int width,
                               int height)
{
    DM1_FloorOrnamentRenderPlanPc34 plan;
    int ok = dm1_v1_floor_ornament_source_zone_pc34(
        relForward, relSide, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("source.dstX", plan.blit.dstX, dstX);
    expect_int("source.dstY", plan.blit.dstY, dstY);
    expect_int("source.width", plan.blit.width, width);
    expect_int("source.height", plan.blit.height, height);
}

int main(void)
{
    DM1_FloorOrnamentRenderPlanPc34 plan;

    /* ReDMCSB DUNVIEW.C F0108 uses G0195 to choose the G0206 coordinate
     * set and G0191-like bitmap increments for visible floor slots. */
    expect_int("plan.count", dm1_v1_floor_ornament_plan_count_pc34(), 9);

    expect_source_zone("source.d3l", 3, -1,  32, 66, 48,  6);
    expect_source_zone("source.d3c", 3,  0,  96, 66, 32,  6);
    expect_source_zone("source.d2c", 2,  0,  80, 77, 64, 11);
    expect_source_zone("source.d1c", 1,  0,  80, 92, 64, 25);

    expect_plan("plan.d3l.global0", 3, -1, 0, 0, 0, 385, 36, 66, 40, 6);
    expect_plan("plan.d3c.global0", 3,  0, 0, 1, 0, 386, 99, 66, 26, 6);
    expect_plan("plan.d2r.global0", 2,  1, 0, 2, 1, 387, 162, 77, 60, 11);
    expect_plan("plan.d1c.global0", 1,  0, 0, 5, 0, 390, 81, 93, 62, 23);
    expect_plan("plan.footprints.d1l", 1, -1, 15, 4, 0, 383, 3, 94, 25, 21);

    expect_int("graphic.global0.inc0",
               dm1_v1_floor_ornament_graphic_index_pc34(0, 0), 385);
    expect_int("graphic.global2.inc5",
               dm1_v1_floor_ornament_graphic_index_pc34(2, 5), 402);
    expect_int("graphic.footprints.inc0",
               dm1_v1_floor_ornament_graphic_index_pc34(15, 0), 379);
    expect_int("graphic.footprints.inc5",
               dm1_v1_floor_ornament_graphic_index_pc34(15, 5), 384);
    expect_int("graphic.bad.global",
               dm1_v1_floor_ornament_graphic_index_pc34(-1, 0), -1);
    expect_int("graphic.bad.increment",
               dm1_v1_floor_ornament_graphic_index_pc34(0, 6), -1);

    expect_int("plan.bad.rel",
               dm1_v1_floor_ornament_render_plan_pc34(4, 0, 0, &plan), 0);
    expect_int("plan.bad.global",
               dm1_v1_floor_ornament_render_plan_pc34(1, 0, -1, &plan), 0);
    expect_int("plan.null",
               dm1_v1_floor_ornament_render_plan_pc34(1, 0, 0, 0), 0);
    expect_int("plan_at.bad",
               dm1_v1_floor_ornament_render_plan_at_pc34(9, 0, &plan), 0);
    expect_int("source.null",
               dm1_v1_floor_ornament_source_zone_pc34(1, 0, 0), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
