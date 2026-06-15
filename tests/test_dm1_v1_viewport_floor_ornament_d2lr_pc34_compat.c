#include "dm1_v1_viewport_floor_ornament_d2lr_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void test_d2l_corridor_floor_ornament_blit_contract(void)
{
    DM1_V1_FloorOrnamentD2LRInputPc34 in = {
        DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34,
        4,
        0,
        250
    };
    DM1_V1_FloorOrnamentD2LRResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0119 line 7020 and F0108 lines 3959-3966. */
    expect_int("d2l.resolve",
               dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0119 line 7020 reads L0208_ai_SquareAspect[M558]. */
    expect_int("d2l.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0119 line 7020 calls F0108 on the corridor path at line 7016. */
    expect_int("d2l.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3959 draws only for a nonzero ordinal. */
    expect_int("d2l.draws", out.draws_floor_ornament ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 decrements the ordinal to a zero-based index. */
    expect_int("d2l.index", out.floor_ornament_index, 3);
    /* ReDMCSB: DUNVIEW.C lines 793-802 and F0108 line 3965 add D2L increment 2. */
    expect_int("d2l.native_bitmap", out.native_bitmap_index, 252);
    /* ReDMCSB: DUNVIEW.C line 1172 gives D2L coordinate-set 0 source X1. */
    expect_int("d2l.source_x", out.source_x, 0);
    /* ReDMCSB: DUNVIEW.C line 1172 gives D2L coordinate-set 0 byte width. */
    expect_int("d2l.source_byte_width", out.source_byte_width, 32);
    /* ReDMCSB: DUNVIEW.C line 3989 passes C10_COLOR_FLESH as transparent color. */
    expect_int("d2l.transparent", out.transparent_color, 10);
    /* ReDMCSB: DUNVIEW.C line 3995 uses C1500 + coordinateSet * 9 + M591. */
    expect_int("d2l.zone", out.zone_index, 1500 + 0 * 9 + 3);
    /* ReDMCSB: DUNVIEW.C line 3967 flips D2R, not M591_VIEW_FLOOR_D2L. */
    expect_int("d2l.flip", out.flip_horizontal ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C line 7020 is not the C02 pit fallthrough at lines 7005-7015. */
    expect_int("d2l.not_open_pit_bug64_path", out.open_pit_bug64_path ? 1 : 0, 0);
}

static void test_transparent_pixel_preserves_side_wall_band(void)
{
    DM1_V1_FloorOrnamentD2LRInputPc34 in = {
        DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34,
        1,
        0,
        100
    };
    DM1_V1_FloorOrnamentD2LRResultPc34 out;
    const unsigned char side_wall_pixel = 0x55;
    const unsigned char ornament_pixel = 0x23;

    /* ReDMCSB: DUNVIEW.C F0108 line 3989 blits with transparent color C10. */
    expect_int("clean.resolve",
               dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989 preserves destination where source is C10. */
    expect_int("clean.transparent_pixel",
               dm1_v1_viewport_floor_ornament_d2lr_blit_pixel_pc34(
                   &out, side_wall_pixel, DM1_V1_PC34_FLOOR_ORNAMENT_TRANSPARENT_COLOR),
               side_wall_pixel);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989 writes non-C10 source pixels. */
    expect_int("clean.ornament_pixel",
               dm1_v1_viewport_floor_ornament_d2lr_blit_pixel_pc34(
                   &out, side_wall_pixel, ornament_pixel),
               ornament_pixel);
    /* ReDMCSB: DUNVIEW.C F0119 lines 6947 and 7020 share the D2L side band, with F0108 line 3989 transparency preserving it. */
    expect_int("clean.side_wall_band", out.side_wall_band_stays_clean ? 1 : 0, 1);
}

static void test_wall_with_side_ornament_does_not_call_f0108(void)
{
    DM1_V1_FloorOrnamentD2LRInputPc34 in = {
        DM1_V1_D2LR_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34,
        DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34,
        7,
        0,
        400
    };
    DM1_V1_FloorOrnamentD2LRResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0119 lines 6945-6973 wall path returns before F0108. */
    expect_int("wall.resolve",
               dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0119 lines 6968-6969 draw wall ornaments instead of reading M558. */
    expect_int("wall.no_floor_flag_read", out.reads_floor_ornament_flag ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0119 line 6973 returns before the F0108 call at line 7020. */
    expect_int("wall.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0119 lines 6947 and 6973 leave the side-wall band clean. */
    expect_int("wall.side_wall_band", out.side_wall_band_stays_clean ? 1 : 0, 1);
}

static void test_d2r_uses_right_floor_source_and_flip(void)
{
    DM1_V1_FloorOrnamentD2LRInputPc34 in = {
        DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D2LR_FLOOR_VIEW_D2R_PC34,
        2,
        0,
        300
    };
    DM1_V1_FloorOrnamentD2LRResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0120 line 7213 calls F0108 with M593_VIEW_FLOOR_D2R. */
    expect_int("d2r.resolve",
               dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C line 1174 gives D2R coordinate-set 0 source X1. */
    expect_int("d2r.source_x", out.source_x, 160);
    /* ReDMCSB: DUNVIEW.C line 1174 gives D2R coordinate-set 0 byte width. */
    expect_int("d2r.source_byte_width", out.source_byte_width, 32);
    /* ReDMCSB: DUNVIEW.C F0108 line 3967 flips M593_VIEW_FLOOR_D2R. */
    expect_int("d2r.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C line 3995 uses C1500 + coordinateSet * 9 + M593. */
    expect_int("d2r.zone", out.zone_index, 1500 + 0 * 9 + 5);
}

static void test_zero_ordinal_calls_f0108_but_draws_nothing(void)
{
    DM1_V1_FloorOrnamentD2LRInputPc34 in = {
        DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34,
        0,
        0,
        250
    };
    DM1_V1_FloorOrnamentD2LRResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0119 line 7020 still calls F0108 with M558. */
    expect_int("zero.resolve",
               dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0119 line 7020 reads M558 before F0108 line 3959 tests zero. */
    expect_int("zero.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0119 line 7020 calls F0108 even when M558 is zero. */
    expect_int("zero.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3959 skips the blit for ordinal zero. */
    expect_int("zero.draws", out.draws_floor_ornament ? 1 : 0, 0);
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = dm1_v1_viewport_floor_ornament_d2lr_source_lock_pc34();

    /* ReDMCSB: DUNVIEW.C F0119 lines 6945-6973 and 7016-7020. */
    expect_int("evidence.d2l_paths", strstr(e, "6945-6973") != NULL &&
               strstr(e, "7016-7020") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3966. */
    expect_int("evidence.f0108_gate", strstr(e, "3959-3966") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989. */
    expect_int("evidence.transparent", strstr(e, "3989") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C lines 1172 and 1174 are within 1167-1195. */
    expect_int("evidence.coordinates", strstr(e, "1167-1195") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 2742-2744. */
    expect_int("evidence.defs", strstr(e, "2742-2744") != NULL, 1);
}

int main(void)
{
    test_d2l_corridor_floor_ornament_blit_contract();
    test_transparent_pixel_preserves_side_wall_band();
    test_wall_with_side_ornament_does_not_call_f0108();
    test_d2r_uses_right_floor_source_and_flip();
    test_zero_ordinal_calls_f0108_but_draws_nothing();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_floor_ornament_d2lr_pc34_compat failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_floor_ornament_d2lr_pc34_compat\n");
    return 0;
}
