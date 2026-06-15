/*
 * DM1 V1 chest open object stack-split source-locked contract gate.
 *
 * Lane: object stack split/merge. This lane pins the F0333 "split" half
 * of the chest open/close round-trip; the F0334 "merge" half is covered
 * by `dm1_v1_chest_close_stack_merge_pc34_compat`. Together they pin
 * that F0333 and F0334 are exact inverses on the visible-window +
 * hidden-tail chain shape.
 *
 * Source anchors (ReDMCSB):
 *   - CHEST.C F0333:30-67 open path
 *   - CHEST.C F0334:79-130 close path (round-trip inverse)
 *   - DUNGEON.C F0159:1664-1681 get-next-thing
 *   - DUNGEON.C F0163:1769-1838 list-append
 *   - DUNGEON.C F0140:1114-1120 container weight
 *   - DEFS.H C0xFFFE_THING_ENDOFLIST, C0xFFFF_THING_NONE, M569_PANEL_CHEST
 *
 * Source-locked contract-only; no real-asset or original-DOS pixel
 * parity claim.
 */

#include "firestaff/dm1/v1/chest/open_stack_split_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label,
                      int got,
                      int want,
                      const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_str(const char *label,
                      const char *got,
                      const char *want,
                      const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               anchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, anchor);
    return 1;
}

static int test_anchor_strings(
    const DM1_V1_ChestOpenStackSplitSpecPc34 *spec)
{
    const DM1_V1_ChestOpenStackSplitAnchorsPc34 *a = &spec->anchors;
    int ok = 1;

    ok &= expect_str("contract marker", spec->contract_marker,
                     "Source-locked contract gate; no real-asset parity claim.",
                     a->f0333_open);
    ok &= expect_int("contract-only flag", spec->contract_only, 1,
                     a->f0333_open);
    ok &= expect_int("chest slot count", spec->chest_slot_count,
                     DM1_PC34_CHEST_SLOT_COUNT, a->f0333_open);
    ok &= expect_int("thing END sentinel", spec->thing_end,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_END,
                     a->chain_end_sentinel);
    ok &= expect_int("thing NONE sentinel", spec->thing_none,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE,
                     a->chain_none_sentinel);
    ok &= expect_int("panel chest", spec->panel_chest,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST,
                     a->panel_content);
    ok &= expect_int("chest thing", spec->chest_thing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
                     a->f0333_open);
    ok &= expect_int("chain length empty", spec->chain_length_empty, 0,
                     a->f0333_open);
    ok &= expect_int("chain length partial", spec->chain_length_partial, 5,
                     a->f0333_open);
    ok &= expect_int("chain length exact", spec->chain_length_exact, 8,
                     a->f0333_open);
    ok &= expect_int("chain length over", spec->chain_length_over, 12,
                     a->f0333_open);
    ok &= expect_int("overflow tail count", spec->overflow_tail_count, 4,
                     a->f0333_open);
    ok &= expect_int("partial visible count", spec->partial_visible_count, 5,
                     a->f0333_open);
    ok &= expect_int("partial tail fill count",
                     spec->partial_tail_fill_count, 3, a->f0333_open);
    ok &= expect_int("exact visible count", spec->exact_visible_count, 8,
                     a->f0333_open);
    ok &= expect_int("exact tail fill count", spec->exact_tail_fill_count, 0,
                     a->f0333_open);
    ok &= expect_int("over visible count", spec->over_visible_count, 8,
                     a->f0333_open);
    ok &= expect_int("over tail fill count", spec->over_tail_fill_count, 0,
                     a->f0333_open);
    ok &= expect_int("round trip visible count",
                     spec->round_trip_visible_count, 8, a->f0334_close);
    ok &= expect_int("round trip hidden tail count",
                     spec->round_trip_hidden_tail_count, 4,
                     a->f0334_close);
    return ok;
}

static int test_empty_chain_open(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: chain walk via F0159, "
        "8-item cap, G0425_aT_ChestSlots writes, C0xFFFE_THING_ENDOFLIST "
        "stop, C0xFFFF_THING_NONE tail fill";
    int ok = 1;
    int i;

    ok &= expect_int("empty open result", p->empty_open_result, 1, a_f0333);
    ok &= expect_int("empty panel content", p->empty_panel_content,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST, a_f0333);
    ok &= expect_int("empty chest thing", p->empty_chest_thing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING, a_f0333);
    ok &= expect_int("empty visible count", p->empty_visible_count, 0,
                     a_f0333);
    ok &= expect_int("empty tail fill count", p->empty_tail_fill_count, 8,
                     a_f0333);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("empty tail fill NONE", p->empty_visible_types[i],
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE,
                         a_f0333);
    }
    return ok;
}

static int test_partial_chain_open(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: chain head from "
        "Container->Slot, F0159 walk, 8-item cap";
    int ok = 1;
    int i;

    ok &= expect_int("partial open result", p->partial_open_result, 1,
                     a_f0333);
    ok &= expect_int("partial visible count", p->partial_visible_count, 5,
                     a_f0333);
    ok &= expect_int("partial tail fill count", p->partial_tail_fill_count, 3,
                     a_f0333);
    ok &= expect_int("partial chain head", p->partial_chain_head_ordinal,
                     0x901, a_f0333);
    /* Visible window G0425[0..4] = V1..V5 in chain order. */
    ok &= expect_int("partial V1", p->partial_visible_types[0], 0x901,
                     a_f0333);
    ok &= expect_int("partial V2", p->partial_visible_types[1], 0x902,
                     a_f0333);
    ok &= expect_int("partial V3", p->partial_visible_types[2], 0x903,
                     a_f0333);
    ok &= expect_int("partial V4", p->partial_visible_types[3], 0x904,
                     a_f0333);
    ok &= expect_int("partial V5", p->partial_visible_types[4], 0x905,
                     a_f0333);
    /* Tail fill G0425[5..7] = NONE. */
    for (i = 5; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("partial NONE tail", p->partial_visible_types[i],
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE,
                         a_f0333);
    }
    return ok;
}

static int test_exact_chain_open(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: chain walk fills the 8 visible "
        "G0425 slots in chain order; no tail fill when chain length == 8";
    int ok = 1;
    int i;

    ok &= expect_int("exact open result", p->exact_open_result, 1, a_f0333);
    ok &= expect_int("exact visible count", p->exact_visible_count, 8,
                     a_f0333);
    ok &= expect_int("exact tail fill count", p->exact_tail_fill_count, 0,
                     a_f0333);
    ok &= expect_int("exact chain head", p->exact_chain_head_ordinal, 0x901,
                     a_f0333);
    /* Visible window G0425[0..7] = V1..V8 in chain order. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("exact visible order",
                         p->exact_visible_types[i], 0x901 + i, a_f0333);
    }
    return ok;
}

static int test_overfull_chain_open(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: 8-item cap break on "
        "L1019_i_ThingCount > 8; F0333 fills G0425[0..7] in chain order "
        "and leaves the 9th+ items reachable via F0159 from the 8th";
    const char *a_f0159 =
        "ReDMCSB DUNGEON.C F0159 lines 1664-1681: returns the Next field "
        "of the carried thing's GENERIC record verbatim";
    int ok = 1;
    int i;

    ok &= expect_int("over open result", p->over_open_result, 1, a_f0333);
    ok &= expect_int("over visible count", p->over_visible_count, 8, a_f0333);
    ok &= expect_int("over tail fill count", p->over_tail_fill_count, 0,
                     a_f0333);
    ok &= expect_int("over chain head", p->over_chain_head_ordinal, 0x901,
                     a_f0333);
    /* Visible window G0425[0..7] = V1..V8 in chain order. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("over visible order",
                         p->over_visible_types[i], 0x901 + i, a_f0333);
    }
    /* Hidden tail reachability: 8th item's Next is the 9th item, and
     * the F0159 walk from the 8th reaches the 9th, 10th, 11th, 12th
     * in chain order. */
    ok &= expect_int("over eighth ordinal", p->over_eighth_ordinal, 0x908,
                     a_f0159);
    ok &= expect_int("over 8th Next -> 9th", p->over_eighth_next_to_ninth, 1,
                     a_f0159);
    ok &= expect_int("over 9th -> 10th", p->over_ninth_next_to_tenth, 1,
                     a_f0159);
    ok &= expect_int("over 10th -> 11th", p->over_tenth_next_to_eleventh, 1,
                     a_f0159);
    ok &= expect_int("over 11th -> 12th", p->over_eleventh_next_to_twelfth, 1,
                     a_f0159);
    ok &= expect_int("over 12th terminator", p->over_twelfth_terminator, 1,
                     a_f0159);
    ok &= expect_int("over hidden tail count", p->over_hidden_tail_count, 4,
                     a_f0159);
    /* The hidden tail items in chain order are 9th, 10th, 11th, 12th. */
    ok &= expect_int("over hidden tail[0] = 9th",
                     p->over_hidden_tail_types[0], 0x909, a_f0159);
    ok &= expect_int("over hidden tail[1] = 10th",
                     p->over_hidden_tail_types[1], 0x90A, a_f0159);
    ok &= expect_int("over hidden tail[2] = 11th",
                     p->over_hidden_tail_types[2], 0x90B, a_f0159);
    ok &= expect_int("over hidden tail[3] = 12th",
                     p->over_hidden_tail_types[3], 0x90C, a_f0159);
    return ok;
}

static int test_round_trip(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: F0333 and F0334 are exact "
        "inverses on the visible-window + hidden-tail chain shape";
    const char *a_f0334 =
        "ReDMCSB CHEST.C F0334 lines 79-130: F0334 close-rewire reads the "
        "F0333 visible window + hidden tail and rewrites the chain in "
        "chain order; F0333 reopen on the rewire re-derives the same "
        "visible window + hidden tail";
    const char *a_f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1769-1838: list-append used by "
        "F0334 to relink the chain in the F0333 split -> F0334 merge "
        "round-trip";
    int ok = 1;
    int i;

    ok &= expect_int("round trip close result", p->round_trip_close_result, 8,
                     a_f0334);
    ok &= expect_int("round trip reopen result", p->round_trip_reopen_result,
                     1, a_f0333);
    ok &= expect_int("round trip visible count",
                     p->round_trip_reopen_visible_count, 8, a_f0333);
    /* Reopen visible window matches the first 8 in chain order. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("round trip visible order",
                         p->round_trip_reopen_visible_types[i],
                         0x901 + i, a_f0333);
    }
    ok &= expect_int("round trip panel content",
                     p->round_trip_reopen_panel_content,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST, a_f0333);
    ok &= expect_int("round trip open chest thing",
                     p->round_trip_reopen_open_chest_thing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING, a_f0333);
    ok &= expect_int("round trip eighth ordinal",
                     p->round_trip_reopen_eighth_ordinal, 0x908, a_f0333);
    /* Round-trip hidden tail: the F0333 reopen on the F0334 rewire
     * produces the same hidden tail as the original F0333 split. */
    ok &= expect_int("round trip hidden tail count",
                     p->round_trip_reopen_hidden_tail_count, 4, a_f0333);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL; ++i) {
        ok &= expect_int("round trip hidden tail order",
                         p->round_trip_reopen_hidden_tail_types[i],
                         0x909 + i, a_f0333);
    }
    /* Round-trip identity: chain head, body order, and hidden tail
     * are preserved across open + close + reopen. */
    ok &= expect_int("round trip chain head preserved",
                     p->round_trip_chain_head_preserved, 1, a_f0334);
    ok &= expect_int("round trip chain order preserved",
                     p->round_trip_chain_order_preserved, 1, a_f0334);
    ok &= expect_int("round trip hidden tail preserved",
                     p->round_trip_hidden_tail_preserved, 1, a_f0163);
    return ok;
}

static int test_open_path_invariants(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: F0333 does not call F0163 "
        "(no relink) or F0334 (no rewire) during the open path";
    const char *a_panel =
        "ReDMCSB DEFS.H:3005-3008 M569_PANEL_CHEST = 6; F0333 line 28 "
        "sets G0424_i_PanelContent to M569_PANEL_CHEST before the same-"
        "open return at lines 31-32";
    int ok = 1;

    ok &= expect_int("F0163 not called during open",
                     p->f0163_not_called_during_open, 1, a_f0333);
    ok &= expect_int("F0334 not called during open",
                     p->f0334_not_called_during_open, 1, a_f0333);
    ok &= expect_int("panel content after open",
                     p->panel_content_after_open,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST, a_panel);
    ok &= expect_int("panel content after close",
                     p->panel_content_after_close,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST, a_panel);
    ok &= expect_int("panel content after reopen",
                     p->panel_content_after_reopen,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST, a_panel);
    return ok;
}

static int test_idempotent_open(const DM1_V1_ChestOpenStackSplitProbePc34 *p)
{
    const char *a_f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-67: F0333 is idempotent on the "
        "visible window + chain head when the same chest is opened twice";
    int ok = 1;
    int i;

    ok &= expect_int("idempotent open result", p->idempotent_open_result, 1,
                     a_f0333);
    ok &= expect_int("idempotent visible count", p->idempotent_visible_count,
                     8, a_f0333);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("idempotent visible order",
                         p->idempotent_visible_types[i], 0x901 + i, a_f0333);
    }
    ok &= expect_int("idempotent chain head", p->idempotent_chain_head_ordinal,
                     0x901, a_f0333);
    ok &= expect_int("idempotent hidden tail count",
                     p->idempotent_hidden_tail_count, 4, a_f0333);
    return ok;
}

int main(void)
{
    DM1_V1_ChestOpenStackSplitProbePc34 probe;
    const DM1_V1_ChestOpenStackSplitSpecPc34 *spec =
        dm1_v1_chest_open_stack_split_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_open_stack_split_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_open_stack_split_source_evidence_pc34());
    if (!dm1_v1_chest_open_stack_split_run_pc34(&probe)) {
        printf("FAIL dm1_v1_chest_open_stack_split_run_pc34 returned 0\n");
        return 1;
    }
    ok &= test_anchor_strings(spec);
    ok &= test_empty_chain_open(&probe);
    ok &= test_partial_chain_open(&probe);
    ok &= test_exact_chain_open(&probe);
    ok &= test_overfull_chain_open(&probe);
    ok &= test_round_trip(&probe);
    ok &= test_open_path_invariants(&probe);
    ok &= test_idempotent_open(&probe);
    printf("assertionCount=%d\n", g_assertions);
    printf("dm1V1ChestOpenStackSplitInvariantOk=%d\n",
           ok && g_failures == 0);
    return ok && g_failures == 0 ? 0 : 1;
}
