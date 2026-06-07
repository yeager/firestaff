#include "csb_v1_viewport_custom_backgrounds_both_backdrops_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_nonempty(
    const char *label,
    const char *value,
    const char *anchor)
{
    ++g_assertions;
    if (!value || value[0] == '\0') {
        ++g_failures;
        printf("FAIL %s empty anchor=%s\n", label, anchor);
        return 0;
    }
    printf("PASS %s nonempty anchor=%s\n", label, anchor);
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
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int test_contract_metadata(
    const CSB_V1_CustomBackgroundsBothBackdropsContract *contract)
{
    int ok = 1;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 fixes viewport ordering,
     * with F0098 lines 2962-3002 providing the base before the CSB-lineage
     * Viewport.cpp 6840/6841 CustomBackgrounds room dispatches. */
    ok &= expect_int("contract.contract_only", contract->contract_only, 1,
                     contract->redmcsb_f0128_anchor);
    ok &= expect_int("first.room", contract->first_room_num, 0,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first.relative_forward",
                     contract->first_relative_forward, 3,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first.relative_side", contract->first_relative_side, -2,
                     contract->csb_lineage_first_dispatch_anchor);
    ok &= expect_int("first.keep_out_ordinal",
                     contract->first_keep_out_ordinal, 0,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.room", contract->second_room_num, 2,
                     contract->csb_lineage_second_dispatch_anchor);
    ok &= expect_int("second.relative_forward",
                     contract->second_relative_forward, 3,
                     contract->csb_lineage_second_dispatch_anchor);
    ok &= expect_int("second.relative_side",
                     contract->second_relative_side, -1,
                     contract->csb_lineage_second_dispatch_anchor);
    ok &= expect_int("second.keep_out_ordinal",
                     contract->second_keep_out_ordinal, 2,
                     contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("order.first_before_second",
                     contract->first_dispatched_before_second, 1,
                     contract->redmcsb_f0128_anchor);
    ok &= expect_int("distinct.rooms", contract->rooms_distinct, 1,
                     contract->non_overlap_note);
    ok &= expect_int("distinct.sides", contract->sides_distinct, 1,
                     contract->non_overlap_note);
    ok &= expect_int("distinct.keep_out_ordinals",
                     contract->keep_out_ordinals_distinct, 1,
                     contract->non_overlap_note);
    ok &= expect_int("order.f0098_before_either",
                     contract->f0098_drawn_before_either_backdrop, 1,
                     contract->redmcsb_f0098_anchor);
    ok &= expect_int("transparency.c10_across_both",
                     contract->c10_transparency_preserved_across_both, 1,
                     contract->redmcsb_defs_c10_anchor);
    ok &= expect_int("non_overlap.both", contract->both_non_overlapping, 1,
                     contract->non_overlap_note);
    ok &= expect_int("order.f0098_first_then_first_then_second",
                     contract->order_includes_f0098_then_first_then_second, 1,
                     contract->source_summary);

    return ok;
}

static int test_no_cell_routes(
    const CSB_V1_CustomBackgroundsBothBackdropsContract *contract)
{
    int ok = 1;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 keeps the backdrop pass
     * separate from the F0107/F0108/F0111/F0115 cell draw functions. */
    ok &= expect_int("first.no_f0107_f0108_f0111_f0115",
                     contract->first_routes_through_f0107_f0108_f0111_f0115,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.no_f0107_f0108_f0111_f0115",
                     contract->second_routes_through_f0107_f0108_f0111_f0115,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first.no_f0107", contract->first_routes_through_f0107,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first.no_f0108", contract->first_routes_through_f0108,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first.no_f0111", contract->first_routes_through_f0111,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("first.no_f0115", contract->first_routes_through_f0115,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.no_f0107", contract->second_routes_through_f0107,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.no_f0108", contract->second_routes_through_f0108,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.no_f0111", contract->second_routes_through_f0111,
                     0, contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_int("second.no_f0115", contract->second_routes_through_f0115,
                     0, contract->redmcsb_f0128_keep_out_anchor);

    return ok;
}

static int test_anchor_strings(
    const CSB_V1_CustomBackgroundsBothBackdropsContract *contract)
{
    int ok = 1;

    ok &= expect_nonempty("anchor.f0128", contract->redmcsb_f0128_anchor,
                          "ReDMCSB DUNVIEW.C F0128:8318-8542");
    ok &= expect_nonempty("anchor.f0098", contract->redmcsb_f0098_anchor,
                          "ReDMCSB DUNVIEW.C F0098:2962-3002");
    ok &= expect_nonempty("anchor.keep_out",
                          contract->redmcsb_f0128_keep_out_anchor,
                          "ReDMCSB DUNVIEW.C F0128:8337-8339");
    ok &= expect_nonempty("anchor.c10", contract->redmcsb_defs_c10_anchor,
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_nonempty("anchor.first_dispatch",
                          contract->csb_lineage_first_dispatch_anchor,
                          "CSB-lineage Viewport.cpp:6840");
    ok &= expect_nonempty("anchor.second_dispatch",
                          contract->csb_lineage_second_dispatch_anchor,
                          "CSB-lineage Viewport.cpp:6841");
    ok &= expect_nonempty("note.non_overlap", contract->non_overlap_note,
                          contract->redmcsb_f0128_keep_out_anchor);
    ok &= expect_nonempty("note.source_summary", contract->source_summary,
                          contract->redmcsb_f0128_anchor);

    return ok;
}

static int test_required_phrases(
    const CSB_V1_CustomBackgroundsBothBackdropsContract *contract)
{
    static const char *const phrases[] = {
        "contract_only=1",
        "room 0",
        "room 2",
        "side=-2",
        "side=-1",
        "non_overlap_with_second_backdrop=1",
        "DUNVIEW.C F0128",
        "DUNVIEW.C F0098",
        "DUNVIEW.C F0128:8337-8339",
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DEFS.H:2595-2611",
        "Viewport.cpp:6840",
        "Viewport.cpp:6841",
        "Viewport.cpp:6503-6551 CustomBackgrounds",
        "no F0107/F0108/F0111/F0115",
        "first then second",
        "F0098 first"
    };
    int ok = 1;
    size_t i;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 and F0098 lines 2962-3002
     * are recorded in both public evidence strings so this gate remains a
     * source-lock contract rather than a real-asset pixel comparison. */
    for (i = 0; i < sizeof(phrases) / sizeof(phrases[0]); ++i) {
        ok &= expect_contains("non_overlap.required_phrase",
                              contract->non_overlap_note, phrases[i],
                              contract->redmcsb_f0128_anchor);
        ok &= expect_contains("summary.required_phrase",
                              contract->source_summary, phrases[i],
                              contract->redmcsb_f0128_anchor);
    }

    return ok;
}

int main(void)
{
    int ok = 1;
    const CSB_V1_CustomBackgroundsBothBackdropsContract *contract =
        csb_v1_viewport_custom_backgrounds_both_backdrops_contract_pc34();

    printf("probe=csb_v1_viewport_custom_backgrounds_both_backdrops_pc34_compat\n");
    ok &= expect_int("contract.present", contract != NULL, 1,
                     "ReDMCSB DUNVIEW.C F0128:8318-8542");
    if (contract) {
        ok &= test_contract_metadata(contract);
        ok &= test_no_cell_routes(contract);
        ok &= test_anchor_strings(contract);
        ok &= test_required_phrases(contract);
    }

    printf("assertions=%d\n", g_assertions);
    return (ok && g_failures == 0) ? 0 : 1;
}
