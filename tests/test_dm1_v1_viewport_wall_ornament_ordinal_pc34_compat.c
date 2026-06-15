#include "dm1_v1_viewport_wall_ornament_ordinal_pc34_compat.h"

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

static void test_zero_ordinal_returns_without_draw(void)
{
    DM1_V1_WallOrnamentOrdinalInputPc34 in = {
        0, DM1_V1_VIEW_WALL_D2C_FRONT_PC34, 3, 259, true
    };
    DM1_V1_WallOrnamentOrdinalResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0107 lines ~3571-3573. */
    expect_int("zero.resolve",
               dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3571-3573. */
    expect_int("zero.draws", out.draws_ornament ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3571-3573. */
    expect_int("zero.alcove_return", out.returns_alcove ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3571-3573. */
    expect_int("zero.source",
               out.source_lines && strstr(out.source_lines, "3571-3575") != NULL, 1);
}

static void test_ordinal_zone_and_alcove_return(void)
{
    DM1_V1_WallOrnamentOrdinalInputPc34 in = {
        3, DM1_V1_VIEW_WALL_D2C_FRONT_PC34, 4, 300, true
    };
    DM1_V1_WallOrnamentOrdinalResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0107 lines ~3575, ~3587, ~3589, ~3933. */
    expect_int("d2c.resolve",
               dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3575. */
    expect_int("d2c.index", out.wall_ornament_index, 2);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3587; DEFS.H line ~4222. */
    expect_int("d2c.zone", out.zone_index, 1004 + 4 * 15 + 10);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3819-3821. */
    expect_int("d2c.native_increment", out.native_bitmap_index, 301);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3849. */
    expect_int("d2c.scale", out.scale_x32, 21);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3849. */
    expect_int("d2c.palette_depth", out.palette_depth, 2);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3589 and ~3933. */
    expect_int("d2c.returns_alcove", out.returns_alcove ? 1 : 0, 1);
}

static void test_right_side_flip_does_not_increment_native_bitmap(void)
{
    DM1_V1_WallOrnamentOrdinalInputPc34 in = {
        5, DM1_V1_VIEW_WALL_D2R_LEFT_PC34, 1, 350, false
    };
    DM1_V1_WallOrnamentOrdinalResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0107 lines ~3817-3821. */
    expect_int("d2r_left.resolve",
               dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3817. */
    expect_int("d2r_left.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3817-3821. */
    expect_int("d2r_left.native_not_incremented", out.native_bitmap_index, 350);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3587. */
    expect_int("d2r_left.zone", out.zone_index, 1004 + 1 * 15 + 8);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3849. */
    expect_int("d2r_left.palette_depth", out.palette_depth, 2);
}

static void test_far_pc34_wall_uses_d3_scale_and_zone_base(void)
{
    DM1_V1_WallOrnamentOrdinalInputPc34 in = {
        1, DM1_V1_VIEW_WALL_D3R2_LEFT_PC34, 2, 259, false
    };
    DM1_V1_WallOrnamentOrdinalResultPc34 out;

    /* ReDMCSB: DEFS.H lines ~2696-2710; DUNVIEW.C F0107 lines ~3817-3841. */
    expect_int("d3r2.resolve",
               dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DEFS.H line ~2697; DUNVIEW.C F0107 line ~3587. */
    expect_int("d3r2.zone", out.zone_index, 1004 + 2 * 15 + 1);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3817. */
    expect_int("d3r2.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3824-3841. */
    expect_int("d3r2.scale", out.scale_x32, 14);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3841. */
    expect_int("d3r2.palette_depth", out.palette_depth, 3);
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = dm1_v1_viewport_wall_ornament_ordinal_source_lock_pc34();

    /* ReDMCSB: DUNVIEW.C F0107 lines ~3571-3575. */
    expect_int("evidence.ordinal", strstr(e, "3571-3575") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0107 line ~3587. */
    expect_int("evidence.zone", strstr(e, "3587") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3817-3821. */
    expect_int("evidence.flip", strstr(e, "3817-3821") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3849. */
    expect_int("evidence.scale", strstr(e, "3834-3849") != NULL, 1);
    /* ReDMCSB: DEFS.H lines ~2696-2710 and ~4222. */
    expect_int("evidence.defs", strstr(e, "DEFS.H:2696-2710") != NULL &&
               strstr(e, "DEFS.H:4222") != NULL, 1);
}

int main(void)
{
    test_zero_ordinal_returns_without_draw();
    test_ordinal_zone_and_alcove_return();
    test_right_side_flip_does_not_increment_native_bitmap();
    test_far_pc34_wall_uses_d3_scale_and_zone_base();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_wall_ornament_ordinal_pc34_compat failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_wall_ornament_ordinal_pc34_compat\n");
    return 0;
}
