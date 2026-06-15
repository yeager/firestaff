/*
 * DM1 V1 chest close object stack-merge source-locked contract gate.
 *
 * Lane: object stack merge.
 *
 * This contract-only CTest pins the F0334 close path's chain-merge invariants
 * for a sparse G0425 close (6 visible items in 8 slots) with a leader hand
 * that holds a same-kind stackable item. It does NOT claim real-asset parity
 * and it does NOT touch the in-game chest container, the dungeon map, or
 * any M11 runtime surface beyond the inventory helper used to drive the
 * open/close cycle. Source anchors:
 *
 *   - CHEST.C F0333:30-67 open-materialization: same-open return, chain
 *     walk via F0159, eight-item cap, G0425_aT_ChestSlots writes,
 *     C0xFFFE_THING_ENDOFLIST stop, C0xFFFF_THING_NONE tail fill.
 *   - CHEST.C F0334:113-132 close-rewire: no-open return, G0426 clear,
 *     Container->Slot=C0xFFFE_THING_ENDOFLIST clobber, scan eight G0425
 *     entries, skip C0xFFFF_THING_NONE, clear slots, relink via F0163
 *     with CM1_MAPX_NOT_ON_A_SQUARE (i.e. list-append mode) for every
 *     non-empty slot after the first.
 *   - DUNGEON.C F0163:1769-1838 list-append: P0287_T_ThingToLink->Next is
 *     forced to C0xFFFE_THING_ENDOFLIST, then F0159_DUNGEON_GetNextThing
 *     walks P0288_T_ThingInList until C0xFFFE_THING_ENDOFLIST is found, and
 *     the last walked thing's Next is overwritten with P0287_T_ThingToLink.
 *     For the F0334 close path, the first non-empty G0425 slot's thing has
 *     Next set to END by F0334 directly, so every subsequent F0163 walk
 *     terminates after zero hops through the carried previousThing.
 *   - DUNGEON.C F0159:1664-1681 get-next-thing: returns the Next field of
 *     the carried thing's GENERIC record verbatim, used by the F0163 walk.
 *   - CHAMPION.C F0297:243-268 / F0298:270-298 / F0300:511-515 /
 *     F0301:606-660 / F0302:662-713 leader hand and C30+ slot path:
 *     F0334 close must not call any of these, so the leader hand stack
 *     count and charges stay byte-stable through the close.
 *   - OBJECT.C F0032:121-145 / F0033:147-212 type/icon classification:
 *     same-kind hand-stack and same-kind chest-item share an itemType
 *     and so are eligible to interact, but F0334 does not perform the
 *     interaction at close time.
 *   - DUNGEON.C F0140:1114-1120 container weight: 50 base plus recursive
 *     linked content weights while the chain is not END; for an open
 *     chest the visible G0425 contents contribute.
 *   - DEFS.H C30..C37:810-817 / C38:1876-1878 / C537..C544:3906-3913
 *     slot and item ordinals.
 *
 * The merge semantics pinned by this gate are:
 *
 *   1. F0334 walks G0425[0..7] and produces a chain whose head is the
 *      first non-empty G0425 slot, whose body is each subsequent
 *      non-empty G0425 slot appended via F0163 (CM1_MAPX_NOT_ON_A_SQUARE),
 *      and whose terminator is C0xFFFE_THING_ENDOFLIST. NONE entries
 *      are skipped and do not appear in the chain.
 *
 *   2. For an N-non-empty G0425 close, F0163 is called exactly (N-1)
 *      times; the first non-empty slot is the head without an F0163 call
 *      because F0334 writes its Next = END directly. Each F0163 call
 *      carries the immediately previous non-empty G0425 slot's thing
 *      as the previousThing argument; gaps of NONE in G0425 do not
 *      cause the F0163 walk to traverse unrelated things.
 *
 *   3. The F0163 walk terminates after zero hops when the previousThing
 *      has Next = END (which is the case for every F0163 call in the
 *      F0334 close path because the first non-empty slot's thing is
 *      written with Next = END by F0334 itself, and subsequent F0163
 *      calls keep updating the chain terminator to the new tail).
 *
 *   4. F0334 close does not touch the leader hand, so a leader-hand
 *      stackable item of the same itemType as a visible G0425 item is
 *      not auto-merged with the chest chain. The hand stack count and
 *      charges are byte-stable through the close and through the
 *      subsequent reopen.
 *
 *   5. The reopen path (F0333) re-reads the F0334-produced chain in
 *      order, so the G0425 visible order after reopen is identical to
 *      the G0425 visible order before close. Sparse NONE gaps in the
 *      pre-close G0425 do not appear in the post-reopen G0425 (the
 *      reopen fills them in from the chain walk), and the leader hand
 *      still holds the same item.
 */

#include "dm1/dm1_v1_chest_close_stack_merge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label,
                      int got,
                      int want,
                      const char *redmcsb_anchor)
{
    ++g_assertions;
    if (!redmcsb_anchor || redmcsb_anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsb_anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsb_anchor);
    return 1;
}

static int expect_str(const char *label,
                      const char *got,
                      const char *want,
                      const char *redmcsb_anchor)
{
    ++g_assertions;
    if (!redmcsb_anchor || redmcsb_anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsb_anchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsb_anchor);
    return 1;
}

static int test_anchor_strings(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec)
{
    const DM1_V1_ChestCloseStackMergeAnchorsPc34 *a = &spec->anchors;
    int ok = 1;

    ok &= expect_str("contract marker", spec->contract_marker,
                     "Source-locked contract gate; no real-asset parity claim.",
                     a->f0333_open);
    ok &= expect_int("contract-only flag", spec->contract_only, 1, a->f0333_open);
    ok &= expect_int("chest slot count", spec->chest_slot_count,
                     DM1_PC34_CHEST_SLOT_COUNT, a->f0333_open);
    ok &= expect_int("thing NONE sentinel", spec->thing_none,
                     DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
                     a->sentinel_chain);
    ok &= expect_int("thing END sentinel", spec->thing_end,
                     DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END,
                     a->sentinel_chain);
    ok &= expect_int("sparse visible count", spec->sparse_visible_count, 6,
                     a->f0334_close);
    ok &= expect_int("expected F0163 call count",
                     spec->expected_f0163_call_count, 5, a->f0163_append);
    ok &= expect_int("expected f0163 walk hops total",
                     spec->expected_f0163_walk_hops, 0, a->f0163_append);
    return ok;
}

static int test_sparse_open_materials_visible_chain(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec,
    const DM1_V1_ChestCloseStackMergeProbePc34 *probe)
{
    const char *f0333 = spec->anchors.f0333_open;
    const char *f0159 = spec->anchors.f0159_get_next;
    int ok = 1;
    int i;

    ok &= expect_int("sparse open succeeded",
                     probe->sparse_open_result, 1, f0333);
    ok &= expect_int("sparse visible count matches expected",
                     probe->sparse_visible_count, 6, f0333);
    for (i = 0; i < spec->chest_slot_count; ++i) {
        const char *anchor = (i == 0 || i == 2 || i == 4 || i == 5 ||
                              i == 6 || i == 7) ? f0333 : f0159;
        ok &= expect_int("sparse visible slot ordinal",
                         probe->sparse_visible_types[i],
                         spec->sparse_visible_types[i], anchor);
    }
    ok &= expect_int("sparse last visible ordinal",
                     probe->sparse_visible_types[7],
                     spec->sparse_last_visible_ordinal, f0333);
    return ok;
}

static int test_leader_hand_independent_of_close(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec,
    const DM1_V1_ChestCloseStackMergeProbePc34 *probe)
{
    const char *f0297 = spec->anchors.f0297_put_leader_hand;
    const char *f0302 = spec->anchors.f0302_slot;
    const char *f0334 = spec->anchors.f0334_close;
    int ok = 1;

    ok &= expect_int("leader hand stackable itemType before close",
                     probe->leader_hand_item_type_before,
                     spec->leader_hand_stackable_item_type, f0297);
    ok &= expect_int("leader hand stackable charges before close",
                     probe->leader_hand_charges_before,
                     spec->leader_hand_stackable_charges, f0302);
    ok &= expect_int("leader hand same-kind itemType after close",
                     probe->leader_hand_item_type_after,
                     spec->leader_hand_stackable_item_type, f0334);
    ok &= expect_int("leader hand stackable charges after close",
                     probe->leader_hand_charges_after,
                     spec->leader_hand_stackable_charges, f0334);
    ok &= expect_int("leader hand stackable weight after close",
                     probe->leader_hand_weight_after,
                     spec->leader_hand_stackable_weight, f0334);
    return ok;
}

static int test_close_rewire_chain(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec,
    const DM1_V1_ChestCloseStackMergeProbePc34 *probe)
{
    const char *f0334 = spec->anchors.f0334_close;
    const char *f0163 = spec->anchors.f0163_append;
    const char *f0159 = spec->anchors.f0159_get_next;
    const char *f0140 = spec->anchors.f0140_container_weight;
    int ok = 1;
    int i;

    ok &= expect_int("close rewire produced chain head",
                     probe->rewire_head_ordinal,
                     spec->sparse_visible_types[0], f0334);
    ok &= expect_int("rewire chain length",
                     probe->rewire_chain_count,
                     spec->sparse_visible_count, f0334);
    ok &= expect_int("rewire chain terminator",
                     probe->rewire_chain_terminator,
                     spec->thing_end, f0334);
    ok &= expect_int("rewire chain leaked no NONE entries",
                     probe->rewire_chain_leaked_none, 0, f0163);
    for (i = 0; i < spec->sparse_visible_count; ++i) {
        const char *anchor = (i == 0) ? f0334 : f0163;
        ok &= expect_int("rewire chain slot ordinal",
                         probe->rewire_chain_types[i],
                         spec->rewire_expected_types[i], anchor);
    }
    for (i = 0; i < spec->sparse_visible_count - 1; ++i) {
        ok &= expect_int("rewire chain Next field",
                         probe->rewire_chain_next[i],
                         spec->rewire_expected_types[i + 1],
                         i == 0 ? f0334 : f0163);
    }
    ok &= expect_int("rewire chain last Next is END",
                     probe->rewire_chain_next[spec->sparse_visible_count - 1],
                     spec->thing_end, f0163);
    ok &= expect_int("rewire F0163 call count",
                     probe->rewire_f0163_call_count,
                     spec->expected_f0163_call_count, f0163);
    ok &= expect_int("rewire F0163 walk hop total",
                     probe->rewire_f0163_walk_hops,
                     spec->expected_f0163_walk_hops, f0159);
    ok &= expect_int("rewire F0163 thing list starts at G0425[2]",
                     probe->rewire_f0163_first_thing_ordinal,
                     spec->sparse_visible_types[2], f0163);
    ok &= expect_int("rewire F0163 first previousThing is G0425[0]",
                     probe->rewire_f0163_first_previous_ordinal,
                     spec->sparse_visible_types[0], f0163);
    ok &= expect_int("rewire F0163 second thing is G0425[4]",
                     probe->rewire_f0163_second_thing_ordinal,
                     spec->sparse_visible_types[4], f0163);
    ok &= expect_int("rewire F0163 second previousThing is G0425[2]",
                     probe->rewire_f0163_second_previous_ordinal,
                     spec->sparse_visible_types[2], f0163);
    ok &= expect_int("rewire container base weight after close",
                     probe->rewire_container_base_weight,
                     spec->rewire_container_base_weight, f0140);
    return ok;
}

static int test_reopen_round_trip(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec,
    const DM1_V1_ChestCloseStackMergeProbePc34 *probe)
{
    const char *f0333 = spec->anchors.f0333_open;
    const char *f0334 = spec->anchors.f0334_close;
    const char *f0297 = spec->anchors.f0297_put_leader_hand;
    int ok = 1;
    int i;

    ok &= expect_int("reopen succeeded", probe->reopen_result, 1, f0333);
    ok &= expect_int("reopen visible count",
                     probe->reopen_visible_count,
                     spec->sparse_visible_count, f0333);
    for (i = 0; i < spec->chest_slot_count; ++i) {
        const char *anchor = (i == 0 || i == 1 || i == 2 || i == 3 ||
                              i == 4 || i == 5) ? f0333 : f0334;
        ok &= expect_int("reopen G0425 slot ordinal",
                         probe->reopen_types[i],
                         spec->reopen_expected_types[i], anchor);
    }
    ok &= expect_int("reopen leader hand itemType unchanged",
                     probe->reopen_leader_hand_item_type,
                     spec->leader_hand_stackable_item_type, f0297);
    ok &= expect_int("reopen leader hand charges unchanged",
                     probe->reopen_leader_hand_charges,
                     spec->leader_hand_stackable_charges, f0297);
    return ok;
}

static int test_chest_cleared_after_close(
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec,
    const DM1_V1_ChestCloseStackMergeProbePc34 *probe)
{
    const char *f0334 = spec->anchors.f0334_close;
    int ok = 1;
    int i;

    for (i = 0; i < spec->chest_slot_count; ++i) {
        ok &= expect_int("G0425 slot cleared after close",
                         probe->post_close_chest_slot_item_types[i],
                         spec->thing_none, f0334);
    }
    ok &= expect_int("openChestThing cleared after close",
                     probe->post_close_open_chest_thing, 0, f0334);
    return ok;
}

int main(void)
{
    const char *f0333 =
        "ReDMCSB CHEST.C F0333:30-67 open-materialization";
    const char *f0334 =
        "ReDMCSB CHEST.C F0334:113-132 close-rewire";
    int ok = 1;
    DM1_V1_ChestCloseStackMergeProbePc34 probe;
    const DM1_V1_ChestCloseStackMergeSpecPc34 *spec =
        dm1_v1_chest_close_stack_merge_spec_pc34();

    printf("probe=dm1_v1_chest_close_stack_merge_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_close_stack_merge_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_close_stack_merge_run_pc34(&probe),
                     1, f0334);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestCloseStackMergeInvariantOk=0\n");
        return 1;
    }

    ok &= test_anchor_strings(spec);
    ok &= test_sparse_open_materials_visible_chain(spec, &probe);
    ok &= test_leader_hand_independent_of_close(spec, &probe);
    ok &= test_close_rewire_chain(spec, &probe);
    ok &= test_chest_cleared_after_close(spec, &probe);
    ok &= test_reopen_round_trip(spec, &probe);

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestCloseStackMergeInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
