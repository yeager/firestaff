#include "dm1_v1_field_teleporter_effect_pc34_compat.h"

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
                        int dstX,
                        int dstY,
                        int dstW,
                        int dstH,
                        int baseStartUnit,
                        int transparentColor,
                        int maskIndexAndFlip)
{
    DM1_FieldRenderPlanPc34 plan;
    int ok = dm1_v1_field_render_plan_at_pc34(index, &plan);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("plan.relForward", plan.relForward, relForward);
    expect_int("plan.relSide", plan.relSide, relSide);
    expect_int("plan.dstX", plan.dstX, dstX);
    expect_int("plan.dstY", plan.dstY, dstY);
    expect_int("plan.dstW", plan.dstW, dstW);
    expect_int("plan.dstH", plan.dstH, dstH);
    expect_int("plan.baseStartUnit", plan.baseStartUnit, baseStartUnit);
    expect_int("plan.transparentColor", plan.transparentColor, transparentColor);
    expect_int("plan.maskIndexAndFlip", plan.maskIndexAndFlip, maskIndexAndFlip);
}

int main(void)
{
    DM1_FieldRenderPlanPc34 plan;

    expect_int("plan.count", dm1_v1_field_render_plan_count_pc34(), 16);
    expect_plan("plan.d3l2", 0, 3, -2, 0, 25, 36, 49, 0x3f, 0x0a, 0x00);
    expect_plan("plan.d3c", 3, 3, 0, 77, 25, 70, 49, 0x3f, 0x8a, 0xff);
    expect_plan("plan.d2l", 7, 2, -1, 0, 19, 78, 74, 0x3f, 0x0a, 0x03);
    expect_plan("plan.d1c", 11, 1, 0, 32, 9, 160, 111, 0x3d, 0x8a, 0xff);
    expect_plan("plan.d0c", 14, 0, 0, 0, 0, 224, 136, 0x3b, 0x8a, 0xff);
    expect_plan("plan.d0r", 15, 0, 1, 191, 0, 33, 136, 0x3f, 0x0a, 0x85);

    expect_int("square.none", dm1_v1_field_square_is_visible_open_pc34(0), 0);
    expect_int("square.visible.only", dm1_v1_field_square_is_visible_open_pc34(0x04), 0);
    expect_int("square.open.only", dm1_v1_field_square_is_visible_open_pc34(0x08), 0);
    expect_int("square.visible.open", dm1_v1_field_square_is_visible_open_pc34(0x0c), 1);
    expect_int("square.visible.open.extra", dm1_v1_field_square_is_visible_open_pc34(0x6c), 1);
    expect_int("at.bad.negative", dm1_v1_field_render_plan_at_pc34(-1, &plan), 0);
    expect_int("at.bad.end", dm1_v1_field_render_plan_at_pc34(16, &plan), 0);
    expect_int("at.bad.null", dm1_v1_field_render_plan_at_pc34(0, 0), 0);
    expect_int("relative.d3l2.ok", dm1_v1_field_render_plan_for_relative_pc34(3, -2, &plan), 1);
    expect_int("relative.d3l2.x", plan.dstX, 0);
    expect_int("relative.d3l2.w", plan.dstW, 36);
    expect_int("relative.d2r2.ok", dm1_v1_field_render_plan_for_relative_pc34(2, 2, &plan), 1);
    expect_int("relative.d2r2.x", plan.dstX, 216);
    expect_int("relative.d2r2.h", plan.dstH, 52);
    expect_int("relative.bad.side", dm1_v1_field_render_plan_for_relative_pc34(3, -3, &plan), 0);
    expect_int("relative.bad.null", dm1_v1_field_render_plan_for_relative_pc34(3, -2, 0), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
