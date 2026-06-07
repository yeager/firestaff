#include "csb_v1_viewport_custom_backgrounds_first_backdrop_pc34_compat.h"

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
    const CSB_V1_CustomBackgroundsFirstBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34();

    ok &= expect_int("contract.present", contract != NULL, 1,
                     "ReDMCSB DUNVIEW.C F0128:8318-8542");
    if (!contract) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 is the viewport pass anchor;
     * CSB-lineage Viewport.cpp lines 6840-6841 select first room 0 before
     * second/fallback room 2. */
    ok &= expect_int("first_backdrop.contract_only",
                     contract->contract_only, 1,
                     contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("first_backdrop.call_index",
                     contract->first_custom_background_call_index, 0,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first_backdrop.room_num",
                     contract->first_custom_background_room_num, 0,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first_backdrop.second_fallback_room",
                     contract->second_custom_background_fallback_room_num, 2,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first_backdrop.selected_before_second",
                     contract->first_custom_background_selected_before_second,
                     1, contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first_backdrop.second_after_first",
                     contract->second_custom_background_is_fallback_after_first,
                     1, contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first_backdrop.rel_forward",
                     contract->first_relative_forward, 3,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("first_backdrop.rel_side",
                     contract->first_relative_side, -2,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("second_backdrop.rel_forward",
                     contract->second_relative_forward, 3,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("second_backdrop.rel_side",
                     contract->second_relative_side, -1,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("first_backdrop.keep_out_ordinal",
                     contract->first_keep_out_region_ordinal, 0,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second_backdrop.keep_out_ordinal",
                     contract->second_keep_out_region_ordinal, 2,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first_backdrop.keep_out_differs",
                     contract->keep_out_region_differs_from_second_backdrop, 1,
                     contract->non_overlap_note);
    ok &= expect_int("first_backdrop.non_overlap_second_gate",
                     contract->explicitly_non_overlapping_with_second_backdrop_gate,
                     1, contract->non_overlap_note);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 writes floor/ceiling base
     * pixels before CSB CustomBackgrounds. */
    ok &= expect_int("first_backdrop.base_pixels_first",
                     contract->f0098_base_pixels_drawn_first, 1,
                     contract->redmcsb_f0098_base_anchor);
    ok &= expect_int("first_backdrop.keep_out_applies",
                     contract->f0128_backdrop_keep_out_applies, 1,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first_backdrop.c10",
                     contract->c10_transparent_color, 10,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("first_backdrop.c10_preserved",
                     contract->c10_transparency_preserved_for_later_routes, 1,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("first_backdrop.no_f0107",
                     contract->routes_through_f0107, 0,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first_backdrop.no_f0108",
                     contract->routes_through_f0108, 0,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first_backdrop.no_f0111",
                     contract->routes_through_f0111, 0,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first_backdrop.no_f0115",
                     contract->routes_through_f0115, 0,
                     contract->redmcsb_f0128_keep_out_anchor);

    return ok;
}

static int test_custom_background_constants(void)
{
    int ok = 1;
    const CSB_V1_CustomBackgroundsFirstBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34();

    if (!contract) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 supplies the pass shape;
     * CSB-lineage Viewport.cpp lines 322 and 6503-6551 supply the CSB
     * CustomBackgrounds count, skin definition slots, and bitmap sizes. */
    ok &= expect_int("custom_background.count",
                     contract->custom_background_count, 16,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("custom_background.skin_def_graphic_id",
                     contract->skin_def_graphic_id, 1,
                     contract->csb_lineage_default_skin_anchor);
    ok &= expect_int("custom_background.skin_def_min",
                     contract->skin_def_min_bytes, 18,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.large_bitmap_index",
                     contract->large_bitmap_skin_def_index, 0,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.large_mask_index",
                     contract->large_mask_skin_def_index, 4,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.large_bitmap_min",
                     contract->large_bitmap_min_bytes, 7840,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.large_mask_min",
                     contract->large_mask_min_bytes, 64,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.middle_bitmap_index",
                     contract->middle_bitmap_skin_def_index, 2,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.middle_mask_index",
                     contract->middle_mask_skin_def_index, 6,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.middle_bitmap_min",
                     contract->middle_bitmap_min_bytes, 3248,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.middle_mask_min",
                     contract->middle_mask_min_bytes, 64,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.near_bitmap_index",
                     contract->near_bitmap_skin_def_index, 1,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.near_mask_index",
                     contract->near_mask_skin_def_index, 5,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.near_bitmap_min",
                     contract->near_bitmap_min_bytes, 4144,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.near_mask_min",
                     contract->near_mask_min_bytes, 20,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("custom_background.near_room_limit",
                     contract->near_layer_room_num_limit, 5,
                     contract->csb_lineage_first_dispatch_anchor);

    ok &= expect_contains("custom_background.lineage.anchor1853",
                          contract->csb_lineage_required_anchors,
                          "1853-1862",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor1881",
                          contract->csb_lineage_required_anchors,
                          "1881-1888",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor1895",
                          contract->csb_lineage_required_anchors,
                          "1895",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor1899",
                          contract->csb_lineage_required_anchors,
                          "1899",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor1922",
                          contract->csb_lineage_required_anchors,
                          "1922",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor1926",
                          contract->csb_lineage_required_anchors,
                          "1926",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("custom_background.lineage.anchor2614",
                          contract->csb_lineage_required_anchors,
                          "2614-2620",
                          contract->redmcsb_f0128_viewport_anchor);

    return ok;
}

static int test_draw_order(void)
{
    int ok = 1;
    CSB_V1_CustomBackgroundsFirstBackdropStep steps[4];
    CSB_V1_CustomBackgroundsFirstBackdropStep one_step[1];
    const CSB_V1_CustomBackgroundsFirstBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34();
    size_t count;

    if (!contract) {
        return 0;
    }

    memset(steps, 0, sizeof(steps));
    memset(one_step, 0, sizeof(one_step));

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 draws base pixels first;
     * CSB-lineage Viewport.cpp lines 6840-6841 then dispatches first room
     * 0 before the second/fallback room 2. */
    count = csb_v1_viewport_custom_backgrounds_first_backdrop_order_pc34(
        steps, sizeof(steps) / sizeof(steps[0]));
    ok &= expect_size("draw_order.count", count, 4u,
                      contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("draw_order.0.f0098_base",
                     (int)steps[0],
                     CSB_V1_FIRST_BACKDROP_STEP_F0098_BASE_PIXELS,
                     contract->redmcsb_f0098_base_anchor);
    ok &= expect_int("draw_order.1.first_custom_background",
                     (int)steps[1],
                     CSB_V1_FIRST_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("draw_order.2.second_fallback",
                     (int)steps[2],
                     CSB_V1_FIRST_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND_FALLBACK,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("draw_order.3.keep_out",
                     (int)steps[3],
                     CSB_V1_FIRST_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT,
                     contract->redmcsb_f0128_keep_out_anchor);

    count = csb_v1_viewport_custom_backgrounds_first_backdrop_order_pc34(
        one_step, sizeof(one_step) / sizeof(one_step[0]));
    ok &= expect_size("draw_order.truncated_count", count, 4u,
                      contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_int("draw_order.truncated_first",
                     (int)one_step[0],
                     CSB_V1_FIRST_BACKDROP_STEP_F0098_BASE_PIXELS,
                     contract->redmcsb_f0098_base_anchor);
    count = csb_v1_viewport_custom_backgrounds_first_backdrop_order_pc34(
        NULL, 0);
    ok &= expect_size("draw_order.null_count", count, 4u,
                      contract->redmcsb_f0128_viewport_anchor);

    return ok;
}

static int test_non_overlap_and_c10(void)
{
    int ok = 1;
    unsigned char src[8] = { 10, 4, 10, 6, 7, 10, 8, 9 };
    unsigned char dst[8] = { 99, 99, 99, 99, 99, 99, 99, 99 };
    const CSB_V1_CustomBackgroundsFirstBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34();
    int copied;

    if (!contract) {
        return 0;
    }

    /* ReDMCSB: DEFS.H line 2088 fixes C10_COLOR_FLESH. This synthetic gate
     * proves first-backdrop C10 pixels preserve the destination for later
     * F0107/F0108/F0111/F0115 blitters while the backdrop itself does not
     * route through those functions. */
    copied = csb_v1_viewport_custom_backgrounds_first_backdrop_preserve_c10_pc34(
        src, dst, sizeof(src));
    ok &= expect_int("c10.copied_non_c10", copied, 5,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.skip0", dst[0], 99,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.copy1", dst[1], 4,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.skip2", dst[2], 99,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.copy3", dst[3], 6,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.copy4", dst[4], 7,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.skip5", dst[5], 99,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.copy6", dst[6], 8,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.copy7", dst[7], 9,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.null_source",
                     csb_v1_viewport_custom_backgrounds_first_backdrop_preserve_c10_pc34(
                         NULL, dst, sizeof(dst)),
                     -1, contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("c10.null_dest",
                     csb_v1_viewport_custom_backgrounds_first_backdrop_preserve_c10_pc34(
                         src, NULL, sizeof(src)),
                     -1, contract->redmcsb_defs_c10_anchor);

    ok &= expect_contains("non_overlap.note.room0",
                          contract->non_overlap_note, "room 0",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("non_overlap.note.room2",
                          contract->non_overlap_note, "room 2",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("non_overlap.note.side_minus2",
                          contract->non_overlap_note, "side=-2",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("non_overlap.note.side_minus1",
                          contract->non_overlap_note, "side=-1",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("non_overlap.note.explicit",
                          contract->non_overlap_note,
                          "non_overlap_with_second_backdrop=1",
                          contract->redmcsb_f0128_keep_out_anchor);

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const CSB_V1_CustomBackgroundsFirstBackdropContract *contract =
        csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_evidence_pc34();

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
    ok &= expect_contains("evidence.c10", evidence,
                          "DEFS.H:2088 C10_COLOR_FLESH",
                          contract->redmcsb_defs_c10_anchor);
    ok &= expect_contains("evidence.view_squares", evidence,
                          "DEFS.H:2595-2611",
                          contract->redmcsb_defs_view_square_anchor);
    ok &= expect_contains("evidence.numcell", evidence,
                          "Viewport.cpp:322 NUMCELL=16",
                          contract->csb_lineage_default_skin_anchor);
    ok &= expect_contains("evidence.custom_backgrounds", evidence,
                          "6503-6551 CustomBackgrounds",
                          contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_contains("evidence.default_skin", evidence,
                          "6521-6524 default skin fallback",
                          contract->csb_lineage_default_skin_anchor);
    ok &= expect_contains("evidence.first_before_second", evidence,
                          "6840 room 0 before 6841 room 2",
                          contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_contains("evidence.no_f0107", evidence,
                          "no F0107/F0108/F0111/F0115 route",
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_contains("evidence.required_anchors", evidence,
                          "1853-1862,1881-1888,1895,1899,1922,1926,2614-2620",
                          contract->redmcsb_f0128_viewport_anchor);
    ok &= expect_contains("evidence.contract_only", evidence,
                          "contract_only=1",
                          contract->redmcsb_f0128_viewport_anchor);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_custom_backgrounds_first_backdrop_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_custom_backgrounds_first_backdrop_source_evidence_pc34());

    ok &= test_contract_metadata();
    ok &= test_custom_background_constants();
    ok &= test_draw_order();
    ok &= test_non_overlap_and_c10();
    ok &= test_source_evidence();

    printf("assertions=%d\n", g_assertions);
    return ok ? 0 : 1;
}
