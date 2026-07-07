#include "dm1_v1_center_door_render_pc34_compat.h"

#include <stdio.h>

static int g_failed = 0;
static int g_passed = 0;

static void expect_int(const char* name, int got, int expected)
{
    if (got != expected) {
        printf("FAIL %s got=%d expected=%d\n", name, got, expected);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, expected);
    ++g_passed;
}

static void expect_blit(const char* name,
                        int depth,
                        int doorState,
                        int doorVertical,
                        int blitIndex,
                        int srcX,
                        int srcY,
                        int dstX,
                        int dstY,
                        int width,
                        int height)
{
    DM1_CenterDoorBlitPc34 blits[2];
    int count = dm1_v1_center_door_panel_blits_for_cell_pc34(
        depth,
        doorState,
        doorVertical,
        blits);
    expect_int(name, count > blitIndex ? 1 : 0, 1);
    if (count <= blitIndex) {
        return;
    }
    expect_int("blit.srcX", blits[blitIndex].srcX, srcX);
    expect_int("blit.srcY", blits[blitIndex].srcY, srcY);
    expect_int("blit.dstX", blits[blitIndex].dstX, dstX);
    expect_int("blit.dstY", blits[blitIndex].dstY, dstY);
    expect_int("blit.width", blits[blitIndex].width, width);
    expect_int("blit.height", blits[blitIndex].height, height);
}

int main(void)
{
    DM1_CenterDoorRenderPlanPc34 plan;
    DM1_CenterDoorBlitPc34 blits[2];

    expect_int("plan.count", dm1_v1_center_door_render_plan_count_pc34(), 3);

    expect_int("plan.d1c", dm1_v1_center_door_render_plan_for_depth_pc34(0, &plan), 1);
    expect_int("plan.d1c.depth", plan.depthIndex, 0);
    expect_int("plan.d1c.relForward", plan.relForward, 1);
    expect_int("plan.d1c.relSide", plan.relSide, 0);
    expect_int("plan.d1c.frameCount", plan.frameCount, 3);
    expect_int("plan.d1c.frameA.graphic", plan.frameA.graphicIndex, 91);
    expect_int("plan.d1c.frameA.dstX", plan.frameA.dstX, 61);
    expect_int("plan.d1c.closed.graphic", plan.closedPanel.graphicIndex, 248);
    expect_int("plan.d1c.closed.width", plan.closedPanel.width, 96);
    expect_int("plan.d1c.closed.height", plan.closedPanel.height, 86);

    expect_int("plan.d2c", dm1_v1_center_door_render_plan_for_depth_pc34(1, &plan), 1);
    expect_int("plan.d2c.frameCount", plan.frameCount, 3);
    expect_int("plan.d2c.frameB.graphic", plan.frameB.graphicIndex, 88);
    expect_int("plan.d2c.frameB.dstX", plan.frameB.dstX, 65);
    expect_int("plan.d2c.closed.graphic", plan.closedPanel.graphicIndex, 247);

    expect_int("plan.d3c", dm1_v1_center_door_render_plan_for_depth_pc34(2, &plan), 1);
    expect_int("plan.d3c.frameCount", plan.frameCount, 2);
    expect_int("plan.d3c.frameA.graphic", plan.frameA.graphicIndex, 89);
    expect_int("plan.d3c.frameA.dstX", plan.frameA.dstX, 82);
    expect_int("plan.d3c.closed.graphic", plan.closedPanel.graphicIndex, 246);

    expect_blit("d1c.closed", 0, 4, 1, 0, 0, 0, 64, 16, 96, 86);
    expect_blit("d2c.vertical.opening", 1, 2, 1, 0, 0, 29, 80, 24, 64, 32);
    expect_blit("d3c.horizontal.left", 2, 1, 0, 0, 18, 0, 88, 28, 6, 40);
    expect_blit("d3c.horizontal.right", 2, 1, 0, 1, 24, 0, 130, 28, 6, 40);

    expect_int("doorSet0.d1.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 0), 248);
    expect_int("doorSet0.d2.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 1), 247);
    expect_int("doorSet0.d3.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 2), 246);
    expect_int("doorSet2.d1.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(2, 0), 254);
    expect_int("doorSet2.d3.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(2, 2), 252);

    expect_int("bad.depth", dm1_v1_center_door_render_plan_for_depth_pc34(3, &plan), 0);
    expect_int("bad.index", dm1_v1_center_door_render_plan_at_pc34(-1, &plan), 0);
    expect_int("bad.null", dm1_v1_center_door_render_plan_for_depth_pc34(0, 0), 0);
    expect_int("open.panel", dm1_v1_center_door_panel_blits_for_cell_pc34(0, 0, 1, blits), 0);
    expect_int("bad.panel.depth", dm1_v1_center_door_panel_blits_for_cell_pc34(-1, 4, 1, blits), 0);
    expect_int("bad.panel.null", dm1_v1_center_door_panel_blits_for_cell_pc34(0, 4, 1, 0), 0);
    expect_int("bad.graphic.depth",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 3), -1);
    expect_int("bad.graphic.set",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(-1, 0), -1);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
