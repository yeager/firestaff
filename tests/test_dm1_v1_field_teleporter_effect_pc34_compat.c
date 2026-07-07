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
    {
        DM1_FieldBitmapSamplePc34 sample;
        uint8_t fieldPixels[256 * 128];
        uint8_t maskPixels[32 * 16];
        uint8_t pixel = 0;
        int i;
        for (i = 0; i < (int)sizeof(fieldPixels); ++i) {
            fieldPixels[i] = (uint8_t)((i % 251) + 1);
        }
        for (i = 0; i < (int)sizeof(maskPixels); ++i) {
            maskPixels[i] = 1;
        }
        expect_int("sample.d3l2.lookup", dm1_v1_field_render_plan_for_relative_pc34(3, -2, &plan), 1);
        expect_int("sample.d3l2.ok", dm1_v1_field_bitmap_sample_pc34(&plan, 0, 0, 0, 128, 64, 16, 8, &sample), 1);
        expect_int("sample.d3l2.fieldX", sample.fieldX, 112);
        expect_int("sample.d3l2.fieldY", sample.fieldY, 0);
        expect_int("sample.d3l2.maskPresent", sample.maskPresent, 1);
        expect_int("sample.d3l2.maskFlip", sample.maskFlip, 0);
        expect_int("sample.d3l2.maskX", sample.maskX, 0);
        expect_int("sample.d3l2.maskY", sample.maskY, 0);
        expect_int("sample.d2r2.lookup", dm1_v1_field_render_plan_for_relative_pc34(2, 2, &plan), 1);
        expect_int("sample.d2r2.tick0", dm1_v1_field_bitmap_sample_pc34(&plan, 0, 7, 51, 256, 128, 32, 16, &sample), 1);
        expect_int("sample.d2r2.fieldX", sample.fieldX, 247);
        expect_int("sample.d2r2.fieldY", sample.fieldY, 51);
        expect_int("sample.d2r2.maskPresent", sample.maskPresent, 1);
        expect_int("sample.d2r2.maskFlip", sample.maskFlip, 1);
        expect_int("sample.d2r2.maskX", sample.maskX, 3);
        expect_int("sample.d2r2.maskY", sample.maskY, 15);
        expect_int("sample.d2r2.transparent", sample.transparentColor, 0x0a);
        expect_int("sample.d2r2.tick3", dm1_v1_field_bitmap_sample_pc34(&plan, 3, 0, 0, 256, 128, 32, 16, &sample), 1);
        expect_int("sample.d2r2.tick3.fieldX", sample.fieldX, 0);
        expect_int("sample.d2r2.tick3.fieldY", sample.fieldY, 21);
        expect_int("sample.bad.local", dm1_v1_field_bitmap_sample_pc34(&plan, 0, plan.dstW, 0, 256, 128, 32, 16, &sample), 0);
        expect_int("sample.bad.field_size", dm1_v1_field_bitmap_sample_pc34(&plan, 0, 0, 0, 0, 128, 32, 16, &sample), 0);
        expect_int("pixel.d2r2.opaque", dm1_v1_field_bitmap_pixel_pc34(&plan, 0, 7, 51,
                   fieldPixels, 256, 128, 256,
                   maskPixels, 32, 16, 32, &pixel), 1);
        expect_int("pixel.d2r2.value", pixel, fieldPixels[51 * 256 + 247]);
        maskPixels[15 * 32 + 3] = 0;
        expect_int("pixel.d2r2.masked", dm1_v1_field_bitmap_pixel_pc34(&plan, 0, 7, 51,
                   fieldPixels, 256, 128, 256,
                   maskPixels, 32, 16, 32, &pixel), 0);
        maskPixels[15 * 32 + 3] = 1;
        fieldPixels[51 * 256 + 247] = 0x0a;
        expect_int("pixel.d2r2.transparent", dm1_v1_field_bitmap_pixel_pc34(&plan, 0, 7, 51,
                   fieldPixels, 256, 128, 256,
                   maskPixels, 32, 16, 32, &pixel), 0);
        fieldPixels[51 * 256 + 247] = 0x22;
        expect_int("pixel.d2r2.no_mask_asset", dm1_v1_field_bitmap_pixel_pc34(&plan, 0, 7, 51,
                   fieldPixels, 256, 128, 256,
                   NULL, 0, 0, 0, &pixel), 1);
        expect_int("pixel.d2r2.no_mask_value", pixel, 0x22);
    }

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
