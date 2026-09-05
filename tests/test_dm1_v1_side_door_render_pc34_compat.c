#include "dm1_v1_side_door_render_pc34_compat.h"

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

static void expect_plan(const char* name,
                        int index,
                        int relForward,
                        int relSide,
                        int depthIndex,
                        int panelGraphic,
                        int panelSrcX,
                        int panelDstX,
                        int panelDstY,
                        int panelW,
                        int panelH,
                        int frameCount)
{
    DM1_SideDoorRenderPlanPc34 plan;
    int ok = dm1_v1_side_door_render_plan_at_pc34(index, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("plan.relForward", plan.relForward, relForward);
    expect_int("plan.relSide", plan.relSide, relSide);
    expect_int("plan.depthIndex", plan.depthIndex, depthIndex);
    expect_int("plan.panel.graphic", plan.panel.graphicIndex, panelGraphic);
    expect_int("plan.panel.srcX", plan.panel.srcX, panelSrcX);
    expect_int("plan.panel.dstX", plan.panel.dstX, panelDstX);
    expect_int("plan.panel.dstY", plan.panel.dstY, panelDstY);
    expect_int("plan.panel.width", plan.panel.width, panelW);
    expect_int("plan.panel.height", plan.panel.height, panelH);
    expect_int("plan.frameCount", plan.frameCount, frameCount);
}

static void expect_blit(const char* name,
                        int relForward,
                        int relSide,
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
    DM1_SideDoorRenderPlanPc34 plan;
    DM1_SideDoorBlitPc34 blits[2];
    int count;
    int ok = dm1_v1_side_door_render_plan_for_rel_pc34(relForward, relSide, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    count = dm1_v1_side_door_panel_blits_for_draw_pc34(
        &plan,
        doorState,
        doorVertical,
        blits);
    expect_int("blit.count.positive", count > blitIndex, 1);
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
    DM1_SideDoorRenderPlanPc34 plan;
    DM1_SideDoorBlitPc34 blits[2];
    DM1_SideDoorF0111CompositionPc34 composition;
    int i;

    expect_int("plan.count", dm1_v1_side_door_render_plan_count_pc34(), 8);
    expect_plan("plan.d3l2", 0, 3, -2, 2, 246, 35, 0, 28, 9, 38, 0);
    expect_plan("plan.d3l", 2, 3, -1, 2, 246, 1, 30, 29, 43, 38, 2);
    expect_plan("plan.d2r", 5, 2, 1, 1, 247, 0, 164, 23, 60, 59, 1);
    expect_plan("plan.d1r", 7, 1, 1, 0, 248, 0, 192, 18, 32, 86, 1);

    expect_blit("d3l.closed.frame", 3, -1, 5, 1, 0, 0, 0, 24, 28, 48, 40);
    expect_blit("d2l.vertical.opening", 2, -1, 2, 1, 0, 0, 30, 0, 24, 64, 31);
    expect_blit("d1r.horizontal.left", 1, 1, 2, 0, 0, 24, 0, 192, 17, 24, 86);
    expect_blit("d3l2.fallback.opening", 3, -2, 2, 1, 0, 35, 17, 0, 28, 9, 21);

    expect_int("bad.index", dm1_v1_side_door_render_plan_at_pc34(8, &plan), 0);
    expect_int("bad.null", dm1_v1_side_door_render_plan_at_pc34(0, 0), 0);
    expect_int("bad.rel", dm1_v1_side_door_render_plan_for_rel_pc34(0, 0, &plan), 0);
    expect_int("bad.rel.null", dm1_v1_side_door_render_plan_for_rel_pc34(3, -1, 0), 0);
    expect_int("no.open.panel", dm1_v1_side_door_panel_blits_for_draw_pc34(&plan, 0, 1, blits), 0);
    expect_int("bad.panel.null", dm1_v1_side_door_panel_blits_for_draw_pc34(0, 1, 1, blits), 0);
    expect_int("bad.out.null", dm1_v1_side_door_panel_blits_for_draw_pc34(&plan, 1, 1, 0), 0);

    /* Every lateral F0111 caller uses the same complete temporary-bitmap
     * transaction as center doors.  Exercise all eight real zone routes,
     * including the exceptional D3L2/D3R2 slices. */
    for (i = 0; i < dm1_v1_side_door_render_plan_count_pc34(); ++i) {
        expect_int("composition.plan", dm1_v1_side_door_render_plan_at_pc34(i, &plan), 1);
        expect_int("composition.accepted",
                   dm1_v1_side_door_f0111_composition_pc34(
                       &plan, 5, 1, 2, 3, i & 3, &composition, blits), 1);
        expect_int("composition.valid", composition.valid, 1);
        expect_int("composition.origin.x", composition.bitmapOriginX,
                   plan.panel.dstX - plan.panel.srcX);
        expect_int("composition.origin.y", composition.bitmapOriginY,
                   plan.panel.dstY - plan.panel.srcY);
        expect_int("composition.ornament.before.clip",
                   composition.ordinaryOrnamentBeforeClip, 1);
        expect_int("composition.destroyed.before.clip",
                   composition.destroyedMaskBeforeClip, 1);
        expect_int("composition.no.thieves.eye",
                   composition.thievesEyeMaskBeforeClip, 0);
        expect_int("composition.flip.before.clip",
                   composition.wholeBitmapFlipBeforeClip, 1);
        expect_int("composition.flip.mask", composition.flipMask, i & 3);
        expect_int("composition.one.random4", composition.random4Consumed, 1);
    }
    expect_int("composition.open.rejected",
               dm1_v1_side_door_f0111_composition_pc34(
                   &plan, 0, 1, 1, 3, 0, &composition, blits), 0);
    expect_int("composition.no.ornament",
               dm1_v1_side_door_f0111_composition_pc34(
                   &plan, 4, 1, 0, 0, 0, &composition, blits), 1);
    expect_int("composition.no.ornament.flag",
               composition.ordinaryOrnamentBeforeClip, 0);
    expect_int("composition.nonanimated.no.random",
               composition.random4Consumed, 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
