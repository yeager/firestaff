#include "dm1_v1_viewport_floor_ornament_d1l2_d1r2_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void test_d1l2_corridor_floor_ornament_blit_contract(void)
{
    DM1_V1_FloorOrnamentD1L2D1R2InputPc34 in = {
        DM1_V1_D1L2_D1R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1L2_PC34,
        4,
        1,
        250
    };
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 out;
    const unsigned char side_wall_pixel = 0x55;
    const unsigned char ornament_pixel = 0x23;

    /* ReDMCSB: DUNVIEW.C F0122 line 7525 and F0108 lines 3959-3966. */
    expect_int("d1l2.resolve",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0122 line 7525 reads L0214_ai_SquareAspect[M558]. */
    expect_int("d1l2.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0122 line 7525 calls F0108 on the corridor path. */
    expect_int("d1l2.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3959 draws only for a nonzero ordinal. */
    expect_int("d1l2.draws", out.draws_floor_ornament ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 decrements the ordinal to a zero-based index. */
    expect_int("d1l2.index", out.floor_ornament_index, 3);
    /* ReDMCSB: DUNVIEW.C lines 793-802 and F0108 line 3965 add D1 far-wall increment 4. */
    expect_int("d1l2.native_bitmap", out.native_bitmap_index, 254);
    /* ReDMCSB: DEFS.H lines 2745-2747 define M594_VIEW_FLOOR_D1L as 6. */
    expect_int("d1l2.view", out.view_floor_index, 6);
    /* ReDMCSB: DUNVIEW.C line 1184 gives D1L coordinate-set 1 source X1. */
    expect_int("d1l2.source_x", out.source_x, 0);
    /* ReDMCSB: DUNVIEW.C line 1184 gives D1L coordinate-set 1 byte width. */
    expect_int("d1l2.source_byte_width", out.source_byte_width, 32);
    /* ReDMCSB: DUNVIEW.C line 3989 passes C10_COLOR_FLESH as transparent color. */
    expect_int("d1l2.transparent", out.transparent_color, 10);
    /* ReDMCSB: DUNVIEW.C line 3995 uses C1500 + coordinateSet * 9 + M594. */
    expect_int("d1l2.zone", out.zone_index, 1500 + 1 * 9 + 6);
    /* ReDMCSB: DUNVIEW.C line 3967 flips D1R, not M594_VIEW_FLOOR_D1L. */
    expect_int("d1l2.flip", out.flip_horizontal ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C line 7525 is the BUG0_64 open-pit fallthrough contract, not a separate branch here. */
    expect_int("d1l2.not_open_pit_bug64_path", out.open_pit_bug64_path ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989 preserves destination where source is C10. */
    expect_int("d1l2.transparent_pixel",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_blit_pixel_pc34(
                   &out, side_wall_pixel,
                   DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_TRANSPARENT_COLOR),
               side_wall_pixel);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989 writes non-C10 source pixels. */
    expect_int("d1l2.ornament_pixel",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_blit_pixel_pc34(
                   &out, side_wall_pixel, ornament_pixel),
               ornament_pixel);
    /* ReDMCSB: DUNVIEW.C lines 7436-7460 and F0108 line 3989 keep side-wall pixels transparent-clean. */
    expect_int("d1l2.side_wall_band", out.side_wall_band_stays_clean ? 1 : 0, 1);
}

static void test_d1r2_uses_right_floor_source_and_flip(void)
{
    DM1_V1_FloorOrnamentD1L2D1R2InputPc34 in = {
        DM1_V1_D1L2_D1R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1R2_PC34,
        2,
        2,
        300
    };
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0123 line 7693 calls F0108 with M596_VIEW_FLOOR_D1R. */
    expect_int("d1r2.resolve",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C line 1195 gives D1R coordinate-set 2 source X1. */
    expect_int("d1r2.source_x", out.source_x, 208);
    /* ReDMCSB: DUNVIEW.C line 1195 gives D1R coordinate-set 2 source X2. */
    expect_int("d1r2.source_x2", out.source_x2, 223);
    /* ReDMCSB: DUNVIEW.C line 1195 gives D1R coordinate-set 2 byte width. */
    expect_int("d1r2.source_byte_width", out.source_byte_width, 8);
    /* ReDMCSB: DUNVIEW.C F0108 line 3967 flips M596_VIEW_FLOOR_D1R. */
    expect_int("d1r2.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C line 3995 uses C1500 + coordinateSet * 9 + M596. */
    expect_int("d1r2.zone", out.zone_index, 1500 + 2 * 9 + 8);
    /* ReDMCSB: DUNVIEW.C lines 793-802 and F0108 line 3965 use D1R increment 4. */
    expect_int("d1r2.native_bitmap", out.native_bitmap_index, 304);
}

static void test_wall_with_side_ornament_does_not_call_f0108(void)
{
    DM1_V1_FloorOrnamentD1L2D1R2InputPc34 in = {
        DM1_V1_D1L2_D1R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34,
        DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1L2_PC34,
        7,
        0,
        400
    };
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0122 lines 7436-7460 wall path returns before F0108. */
    expect_int("wall.resolve",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0122 line 7459 draws wall ornaments instead of reading M558. */
    expect_int("wall.no_floor_flag_read", out.reads_floor_ornament_flag ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0122 line 7460 returns before the F0108 call at line 7525. */
    expect_int("wall.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0123 lines 7604-7628 mirror the D1R wall-side ornament return. */
    expect_int("wall.side_wall_band", out.side_wall_band_stays_clean ? 1 : 0, 1);
}

static void test_zero_ordinal_calls_f0108_but_draws_nothing(void)
{
    DM1_V1_FloorOrnamentD1L2D1R2InputPc34 in = {
        DM1_V1_D1L2_D1R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1L2_PC34,
        0,
        0,
        250
    };
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0122 line 7525 still calls F0108 with M594. */
    expect_int("zero.resolve",
               dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0122 line 7525 reads M558 before F0108 line 3959 tests zero. */
    expect_int("zero.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0122 line 7525 calls F0108 even when M558 is zero. */
    expect_int("zero.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3959 skips the blit for ordinal zero. */
    expect_int("zero.draws", out.draws_floor_ornament ? 1 : 0, 0);
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = dm1_v1_viewport_floor_ornament_d1l2_d1r2_source_lock_pc34();

    /* ReDMCSB: DUNVIEW.C F0122 lines 7436-7460 and 7520-7525. */
    expect_int("evidence.d1l2_paths", strstr(e, "7436-7460") != NULL &&
               strstr(e, "7520-7525") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0123 lines 7604-7628 and 7688-7693. */
    expect_int("evidence.d1r2_paths", strstr(e, "7604-7628") != NULL &&
               strstr(e, "7688-7693") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3966. */
    expect_int("evidence.f0108_gate", strstr(e, "3959-3966") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965. */
    expect_int("evidence.zero_based", strstr(e, "3965") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3967. */
    expect_int("evidence.flip", strstr(e, "3967") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C lines 793-802. */
    expect_int("evidence.native_increment", strstr(e, "793-802") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C lines 1167-1195. */
    expect_int("evidence.coordinates", strstr(e, "1167-1195") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 2745-2747 and 4223. */
    expect_int("evidence.defs", strstr(e, "2745-2747") != NULL &&
               strstr(e, "4223") != NULL, 1);
    /* ReDMCSB: Source-locked contract warning for this synthetic gate. */
    expect_int("evidence.contract_only", strstr(e, "does not claim full real-asset bitmap parity") != NULL, 1);
}

int main(void)
{
    test_d1l2_corridor_floor_ornament_blit_contract();
    test_d1r2_uses_right_floor_source_and_flip();
    test_wall_with_side_ornament_does_not_call_f0108();
    test_zero_ordinal_calls_f0108_but_draws_nothing();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_floor_ornament_d1l2_d1r2_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_floor_ornament_d1l2_d1r2_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
