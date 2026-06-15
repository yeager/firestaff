#include "csb_v1_viewport_custom_backgrounds_second_backdrop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_size(
    const char *label,
    size_t got,
    size_t want,
    const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%zu want=%zu anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%zu anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int test_contract_metadata(void)
{
    int ok = 1;
    const CSB_V1_CustomBackgroundsSecondBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_second_backdrop_contract_pc34();

    ok &= expect_int("contract.present", contract != NULL, 1,
                     "ReDMCSB DUNVIEW.C F0128:8318-8542");
    if (!contract) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 is the viewport pass anchor;
     * CSBWin Viewport.cpp lines 6919-6920 enumerate room 0, then room 2. */
    ok &= expect_int("contract.contract_only",
                     contract->contract_only, 1,
                     contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("second_backdrop.call_index",
                     contract->second_custom_background_call_index, 1,
                     contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("second_backdrop.room_num",
                     contract->second_custom_background_room_num, 2,
                     contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("second_backdrop.unmasked_baseline",
                     contract->second_custom_background_is_unmasked_baseline, 1,
                     contract->redmcsb_f0128_viewport_anchor);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 writes the floor/ceiling
     * pixels before later CSB CustomBackgrounds layers are considered. */
    ok &= expect_int("second_backdrop.base_pixels_first",
                     contract->f0098_base_pixels_drawn_first, 1,
                     contract->redmcsb_f0098_base_anchor);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 is the explicit backdrop
     * keep-out path and has no CSB near-layer replacement in this contract. */
    ok &= expect_int("second_backdrop.keep_out_applies",
                     contract->f0128_backdrop_keep_out_applies, 1,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second_backdrop.only_masked_no_near_substitute",
                     contract->keep_out_only_masked_overlay_without_near_substitute,
                     1,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second_backdrop.no_csb_near_substitute",
                     contract->has_csb_near_layer_substitute_for_keep_out, 0,
                     contract->redmcsb_f0128_second_backdrop_anchor);

    ok &= expect_contains("second_backdrop.csb_lineage_path.room0",
                          contract->csb_lineage_index_path,
                          "Viewport.cpp:6919",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("second_backdrop.csb_lineage_path.room2",
                          contract->csb_lineage_index_path,
                          "6920 CustomBackgrounds(..., room 2)",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("second_backdrop.source.contract_only",
                          contract->source_summary,
                          "contract_only=1",
                          contract->redmcsb_f0128_second_backdrop_anchor);

    return ok;
}

static int test_draw_order(void)
{
    int ok = 1;
    CSB_V1_CustomBackgroundsSecondBackdropStep steps[4];
    CSB_V1_CustomBackgroundsSecondBackdropStep one_step[1];
    const CSB_V1_CustomBackgroundsSecondBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_second_backdrop_contract_pc34();
    size_t count;

    if (!contract) {
        return 0;
    }

    memset(steps, 0, sizeof(steps));
    memset(one_step, 0, sizeof(one_step));

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 draws base pixels first;
     * CSBWin Viewport.cpp lines 6919-6920 enumerate first and second
     * CustomBackgrounds entries before the F3L1 cell draw path. */
    count = csb_v1_viewport_custom_backgrounds_second_backdrop_order_pc34(
        steps, sizeof(steps) / sizeof(steps[0]));
    ok &= expect_size("draw_order.count", count, 4u,
                      contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("draw_order.0.f0098_base",
                     (int)steps[0],
                     CSB_V1_SECOND_BACKDROP_STEP_F0098_BASE_PIXELS,
                     contract->redmcsb_f0098_base_anchor);
    ok &= expect_int("draw_order.1.first_custom_background",
                     (int)steps[1],
                     CSB_V1_SECOND_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND,
                     contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("draw_order.2.second_custom_background",
                     (int)steps[2],
                     CSB_V1_SECOND_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND,
                     contract->redmcsb_f0128_second_backdrop_anchor);
    ok &= expect_int("draw_order.3.keep_out",
                     (int)steps[3],
                     CSB_V1_SECOND_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT,
                     contract->redmcsb_f0128_keep_out_anchor);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 fixes the viewport pass shape;
     * callers may copy less metadata but still receive the full contract
     * count, matching the existing CustomBackgrounds helper style. */
    count = csb_v1_viewport_custom_backgrounds_second_backdrop_order_pc34(
        one_step, sizeof(one_step) / sizeof(one_step[0]));
    ok &= expect_size("draw_order.truncated_count", count, 4u,
                      contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("draw_order.truncated_first",
                     (int)one_step[0],
                     CSB_V1_SECOND_BACKDROP_STEP_F0098_BASE_PIXELS,
                     contract->redmcsb_f0098_base_anchor);

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const CSB_V1_CustomBackgroundsSecondBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_second_backdrop_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_second_backdrop_source_evidence_pc34();

    if (!contract) {
        return 0;
    }

    ok &= expect_contains("evidence.f0128_viewport", evidence,
                          "F0128:8318-8542",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("evidence.f0098_base", evidence,
                          "F0098:2962-3002",
                          contract->redmcsb_f0098_base_anchor);
    ok &= expect_contains("evidence.f0128_keep_out", evidence,
                          "F0128:8337-8339",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("evidence.csb_lineage_second", evidence,
                          "Viewport.cpp:6919-6920",
                          contract->redmcsb_f0128_second_backdrop_anchor);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_custom_backgrounds_second_backdrop_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_second_backdrop_source_evidence_pc34());

    ok &= test_contract_metadata();
    ok &= test_draw_order();
    ok &= test_source_evidence();

    printf("assertions=%d\n", g_assertions);
    return ok ? 0 : 1;
}
