#include "dm1_v1_door_ornament_render_pc34_compat.h"

#include <stdio.h>

static int g_passed;
static int g_failed;

static void expect_int(const char* name, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", name, got, want);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, want);
    ++g_passed;
}

static void expect_palette(const char* name,
                           const DM1_DoorOrnamentRenderPlanPc34* plan,
                           int valid,
                           int p1,
                           int p6,
                           int p14)
{
    expect_int(name, plan ? plan->paletteMapValid : -1, valid);
    if (!plan || !valid) {
        return;
    }
    expect_int("palette.1", plan->paletteMap[1], p1);
    expect_int("palette.6", plan->paletteMap[6], p6);
    expect_int("palette.14", plan->paletteMap[14], p14);
}

int main(void)
{
    DM1_DoorOrnamentInfoPc34 info;
    DM1_DoorOrnamentPanelPc34 panel;
    DM1_DoorOrnamentRenderPlanPc34 plan;

    expect_int("info.global0", dm1_v1_door_ornament_info_for_global_pc34(0, &info), 1);
    expect_int("info.global0.graphic", info.graphicIndex, 441);
    expect_int("info.global0.coord", info.coordSet, 0);
    expect_int("info.global6", dm1_v1_door_ornament_info_for_global_pc34(6, &info), 1);
    expect_int("info.global6.graphic", info.graphicIndex, 447);
    expect_int("info.global6.coord", info.coordSet, 2);
    expect_int("info.global12", dm1_v1_door_ornament_info_for_global_pc34(12, &info), 1);
    expect_int("info.global12.graphic", info.graphicIndex, 453);
    expect_int("info.global12.default.coord", info.coordSet, 1);
    expect_int("info.bad", dm1_v1_door_ornament_info_for_global_pc34(-1, &info), 0);
    expect_int("info.null", dm1_v1_door_ornament_info_for_global_pc34(0, 0), 0);

    panel.srcY = 0;
    panel.dstX = 64;
    panel.dstY = 16;
    panel.width = 96;
    panel.height = 86;
    expect_int("plan.d1.full", dm1_v1_door_ornament_render_plan_pc34(0, 0, &panel, 16, 19, &plan), 1);
    expect_int("plan.d1.graphic", plan.graphicIndex, 441);
    expect_int("plan.d1.srcY", plan.srcY, 0);
    expect_int("plan.d1.srcH", plan.srcH, 19);
    expect_int("plan.d1.dstX", plan.dstX, 96);
    expect_int("plan.d1.dstY", plan.dstY, 29);
    expect_int("plan.d1.dstW", plan.dstW, 32);
    expect_int("plan.d1.dstH", plan.dstH, 19);
    expect_int("plan.d1.transparent", plan.transparentColor, 9);
    expect_palette("plan.d1.palette", &plan, 0, 0, 0, 0);

    panel.srcY = 21;
    panel.dstX = 64;
    panel.dstY = 16;
    panel.width = 96;
    panel.height = 67;
    expect_int("plan.d1.crop", dm1_v1_door_ornament_render_plan_pc34(0, 0, &panel, 16, 19, &plan), 1);
    expect_int("plan.d1.crop.srcY", plan.srcY, 8);
    expect_int("plan.d1.crop.srcH", plan.srcH, 11);
    expect_int("plan.d1.crop.dstX", plan.dstX, 96);
    expect_int("plan.d1.crop.dstY", plan.dstY, 16);
    expect_int("plan.d1.crop.dstW", plan.dstW, 32);
    expect_int("plan.d1.crop.dstH", plan.dstH, 11);

    panel.srcY = 0;
    panel.dstX = 80;
    panel.dstY = 24;
    panel.width = 64;
    panel.height = 59;
    expect_int("plan.d2.coord2", dm1_v1_door_ornament_render_plan_pc34(6, 1, &panel, 16, 13, &plan), 1);
    expect_int("plan.d2.dstX", plan.dstX, 102);
    expect_int("plan.d2.dstY", plan.dstY, 46);
    expect_int("plan.d2.dstW", plan.dstW, 21);
    expect_int("plan.d2.dstH", plan.dstH, 13);
    expect_palette("plan.d2.palette", &plan, 1, 1, 6, 14);

    panel.srcY = 0;
    panel.dstX = 90;
    panel.dstY = 30;
    panel.width = 44;
    panel.height = 38;
    expect_int("plan.d3", dm1_v1_door_ornament_render_plan_pc34(0, 2, &panel, 8, 10, &plan), 1);
    expect_int("plan.d3.dstX", plan.dstX, 107);
    expect_int("plan.d3.dstY", plan.dstY, 38);
    expect_int("plan.d3.dstW", plan.dstW, 15);
    expect_int("plan.d3.dstH", plan.dstH, 10);
    expect_palette("plan.d3.palette", &plan, 1, 12, 0, 0);

    expect_int("bad.depth", dm1_v1_door_ornament_render_plan_pc34(0, 3, &panel, 8, 10, &plan), 0);
    expect_int("bad.source", dm1_v1_door_ornament_render_plan_pc34(0, 0, &panel, 0, 10, &plan), 0);
    expect_int("bad.panel", dm1_v1_door_ornament_render_plan_pc34(0, 0, 0, 8, 10, &plan), 0);
    expect_int("bad.out", dm1_v1_door_ornament_render_plan_pc34(0, 0, &panel, 8, 10, 0), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
