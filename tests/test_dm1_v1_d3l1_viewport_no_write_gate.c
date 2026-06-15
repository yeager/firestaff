#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_nonnull(const char *id, const void *ptr)
{
    if (!ptr) {
        printf("FAIL %s is NULL\n", id);
        ++g_failures;
    } else {
        printf("PASS %s non-null\n", id);
    }
}

static int square_list_contains(const DM1_ViewSquareIndex *squares,
                                size_t count,
                                DM1_ViewSquareIndex needle)
{
    for (size_t i = 0; i < count; ++i) {
        if (squares[i] == needle) return 1;
    }
    return 0;
}

static int draw_order_index_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_draw_order_count(); ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(i);
        if (step && step->square == square) return (int)i;
    }
    return -1;
}

static void test_d3l1_no_write_spec(void)
{
    static const DM1_ViewSquareIndex expected_allowed[] = {
        DM1_VIEW_SQUARE_D4L,
        DM1_VIEW_SQUARE_D4R,
        DM1_VIEW_SQUARE_D4C,
        DM1_VIEW_SQUARE_D3L2,
        DM1_VIEW_SQUARE_D3R2,
        DM1_VIEW_SQUARE_D3R,
        DM1_VIEW_SQUARE_D3C,
        DM1_VIEW_SQUARE_D2L2,
        DM1_VIEW_SQUARE_D2R2,
        DM1_VIEW_SQUARE_D2L,
        DM1_VIEW_SQUARE_D2R,
        DM1_VIEW_SQUARE_D2C,
        DM1_VIEW_SQUARE_D1L,
        DM1_VIEW_SQUARE_D1R,
        DM1_VIEW_SQUARE_D1C,
        DM1_VIEW_SQUARE_D0L,
        DM1_VIEW_SQUARE_D0R,
        DM1_VIEW_SQUARE_D0C,
    };
    const DM1_ViewportNoWriteSpec *spec =
        dm1_viewport_3d_get_d3l1_no_write_spec();

    /*
     * Source-lock anchors for this gate:
     * - DUNVIEW.C:6361-6495 F0116_DUNGEONVIEW_DrawSquareD3L is the
     *   ordinary DM1 D3L1 square helper.
     * - DUNVIEW.C:6406-6437 routes a D3L wall through the wall/ornament
     *   path and returns before the F0115 thing handoff unless the front
     *   ornament is an alcove.
     * - DUNVIEW.C:8488-8499 F0128 still dispatches the surrounding D3L,
     *   D3R, and D3C draw paths after DUNGEON.C:1371-1421 F0150 resolves
     *   their relative map cells.
     */
    check_nonnull("d3l1_no_write.spec", spec);
    if (!spec) return;

    check_int("d3l1_no_write.target_square",
              (int)spec->target_square, (int)DM1_VIEW_SQUARE_D3L);
    check_int("d3l1_no_write.target_not_allowed",
              square_list_contains(spec->allowed_touch_squares,
                                   spec->allowed_touch_square_count,
                                   DM1_VIEW_SQUARE_D3L),
              0);
    check_int("d3l1_no_write.allowed_count",
              (int)spec->allowed_touch_square_count,
              (int)(sizeof(expected_allowed) / sizeof(expected_allowed[0])));
    check_int("d3l1_no_write.function",
              strcmp(spec->redmcsb_function, "F0116_DUNGEONVIEW_DrawSquareD3L") == 0,
              1);
    check_int("d3l1_no_write.source_lines",
              strstr(spec->source_lines, "DUNVIEW.C:6361-6495") != NULL &&
              strstr(spec->source_lines, "DUNVIEW.C:8488-8499") != NULL &&
              strstr(spec->source_lines, "DUNGEON.C:1371-1421") != NULL,
              1);
    check_int("d3l1_no_write.source_evidence",
              strstr(spec->source_evidence, "DUNVIEW.C:6406-6437") != NULL &&
              strstr(spec->source_evidence, "DUNVIEW.C:6475-6480") != NULL,
              1);

    for (size_t i = 0; i < sizeof(expected_allowed) / sizeof(expected_allowed[0]); ++i) {
        char id[128];
        const DM1_ViewportDrawStep *step;
        snprintf(id, sizeof(id), "d3l1_no_write.allowed.%zu.square", i);
        check_int(id, (int)spec->allowed_touch_squares[i], (int)expected_allowed[i]);

        snprintf(id, sizeof(id), "d3l1_no_write.allowed.%zu.draw_order_present", i);
        check_int(id, draw_order_index_for_square(expected_allowed[i]) >= 0, 1);

        step = dm1_viewport_3d_get_draw_order_step((size_t)draw_order_index_for_square(expected_allowed[i]));
        snprintf(id, sizeof(id), "d3l1_no_write.allowed.%zu.draw_source", i);
        check_int(id, step && strstr(step->source_lines, "DUNVIEW.C:") != NULL, 1);
    }

    check_int("d3l1_no_write.target_draw_order_still_documented",
              draw_order_index_for_square(DM1_VIEW_SQUARE_D3L), 5);
    check_int("d3l1_no_write.d3r_still_invoked",
              draw_order_index_for_square(DM1_VIEW_SQUARE_D3R), 6);
    check_int("d3l1_no_write.d3c_still_invoked",
              draw_order_index_for_square(DM1_VIEW_SQUARE_D3C), 7);
    check_int("d3l1_no_write.d3r_wall_spec",
              dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3R) != NULL, 1);
    check_int("d3l1_no_write.d3c_wall_spec",
              dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3C) != NULL, 1);
    check_int("d3l1_no_write.d2l_wall_spec",
              dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2L) != NULL, 1);
}

static void test_d3l1_source_evidence(void)
{
    const char *evidence = dm1_viewport_3d_source_evidence();
    check_nonnull("d3l1_no_write.source_evidence.global", evidence);
    if (!evidence) return;

    check_int("d3l1_no_write.global_f0116",
              strstr(evidence, "DUNVIEW.C:6361-6495") != NULL &&
              strstr(evidence, "D3L1 no-write target evidence") != NULL, 1);
    check_int("d3l1_no_write.global_f0128",
              strstr(evidence, "DUNVIEW.C:8488-8499") != NULL, 1);
    check_int("d3l1_no_write.global_f0150",
              strstr(evidence, "DUNGEON.C:1371-1421 F0150") != NULL, 1);
}

int main(void)
{
    test_d3l1_no_write_spec();
    test_d3l1_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_d3l1_viewport_no_write_gate failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_d3l1_viewport_no_write_gate\n");
    return 0;
}
