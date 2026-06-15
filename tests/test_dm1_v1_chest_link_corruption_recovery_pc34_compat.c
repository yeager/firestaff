#include "dm1/dm1_v1_chest_link_corruption_recovery_pc34_compat.h"

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

static int expect_contains(const char *label,
                           const char *haystack,
                           const char *needle,
                           const char *redmcsb_anchor)
{
    ++g_assertions;
    if (!redmcsb_anchor || redmcsb_anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing phrase=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsb_anchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsb_anchor);
    return 1;
}

static int test_anchor_strings(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const DM1_V1_ChestLinkCorruptionRecoveryAnchorsPc34 *a = &spec->anchors;
    int ok = 1;

    ok &= expect_contains("anchor F0333 range", a->f0333_open_materialization,
                          "CHEST.C F0333:30-67",
                          a->f0333_open_materialization);
    ok &= expect_contains("anchor F0334 range", a->f0334_close_rewrite,
                          "CHEST.C F0334:113-132",
                          a->f0334_close_rewrite);
    ok &= expect_contains("anchor F0163 range", a->f0163_link_append,
                          "DUNGEON.C F0163:1769-1838",
                          a->f0163_link_append);
    ok &= expect_contains("anchor F0164 range", a->f0164_square_cleanup,
                          "DUNGEON.C F0164:1840-1905",
                          a->f0164_square_cleanup);
    ok &= expect_contains("anchor F0140 range", a->f0140_container_weight,
                          "DUNGEON.C F0140:1114-1120",
                          a->f0140_container_weight);
    ok &= expect_contains("anchor F0297 range", a->f0297_put_leader_hand,
                          "CHAMPION.C F0297:243-268",
                          a->f0297_put_leader_hand);
    ok &= expect_contains("anchor F0298 range", a->f0298_remove_leader_hand,
                          "CHAMPION.C F0298:270-298",
                          a->f0298_remove_leader_hand);
    ok &= expect_contains("anchor F0300 range", a->f0300_clear_c30_slot,
                          "CHAMPION.C F0300:511-515",
                          a->f0300_clear_c30_slot);
    ok &= expect_contains("anchor F0301 range", a->f0301_write_c30_slot,
                          "CHAMPION.C F0301:606-614",
                          a->f0301_write_c30_slot);
    ok &= expect_contains("anchor F0033 range", a->f0033_icon_identity,
                          "OBJECT.C F0033:147-212",
                          a->f0033_icon_identity);
    ok &= expect_contains("anchor F0133 range", a->f0133_partial_mask_dispatch,
                          "BLITMASK.C F0133:30-33",
                          a->f0133_partial_mask_dispatch);
    ok &= expect_contains("anchor sentinel C0xFFFF", a->sentinel_chain,
                          "C0xFFFF_THING_NONE", a->sentinel_chain);
    ok &= expect_contains("anchor sentinel C0xFFFE", a->sentinel_chain,
                          "C0xFFFE_THING_ENDOFLIST", a->sentinel_chain);
    ok &= expect_contains("contract scope marker", a->contract_scope,
                          "contract_only=1", a->contract_scope);
    ok &= expect_contains("no real asset marker", a->contract_scope,
                          "no real-asset parity claim", a->contract_scope);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const char *f0333 = spec->anchors.f0333_open_materialization;
    const char *f0140 = spec->anchors.f0140_container_weight;
    int ok = 1;
    int i;

    ok &= expect_int("contract-only marker", spec->contract_only, 1, f0333);
    ok &= expect_int("no-claim real-asset parity marker",
                     spec->no_real_asset_parity_claim, 1,
                     spec->anchors.f0133_partial_mask_dispatch);
    ok &= expect_int("slot count", spec->slot_count, 8, f0333);
    ok &= expect_int("thing NONE sentinel", spec->thing_none, 0xFFFF,
                     spec->anchors.sentinel_chain);
    ok &= expect_int("thing END sentinel", spec->thing_end, 0xFFFE,
                     spec->anchors.sentinel_chain);
    ok &= expect_int("initial container slot",
                     spec->initial_container_slot,
                     spec->initial_visible_slots[0], f0333);
    ok &= expect_int("initial visible count", spec->initial_visible_count, 8,
                     f0333);
    ok &= expect_int("initial chain terminates at END",
                     spec->initial_chain_terminates_at_end, 1, f0333);
    for (i = 0; i < spec->slot_count; ++i) {
        ok &= expect_int("initial visible slot ordinal",
                         spec->initial_visible_slots[i], 0x210 + i, f0333);
    }
    for (i = 0; i < spec->slot_count - 1; ++i) {
        ok &= expect_int("initial Next ordinal",
                         spec->initial_next_by_item[i],
                         spec->initial_visible_slots[i + 1], f0333);
    }
    ok &= expect_int("initial last Next is END",
                     spec->initial_next_by_item[7], spec->thing_end, f0333);
    ok &= expect_int("container base weight", spec->container_base_weight,
                     50, f0140);
    ok &= expect_int("initial container weight",
                     spec->initial_container_weight,
                     50 + 2 + 3 + 5 + 7 + 11 + 13 + 17 + 19, f0140);
    return ok;
}

static int test_corruption_present(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const char *sentinel = spec->anchors.sentinel_chain;
    const char *f0334 = spec->anchors.f0334_close_rewrite;
    int ok = 1;
    int i;

    ok &= expect_int("all corruptions present", spec->corruptions_present, 1,
                     f0334);
    ok &= expect_int("corrupt null-next item", spec->corrupt_null_next_item,
                     0x212, sentinel);
    ok &= expect_int("corrupt null-next value",
                     spec->corrupted_next_by_item[2], spec->thing_none,
                     sentinel);
    ok &= expect_int("corrupt duplicate item", spec->corrupt_duplicate_item,
                     0x211, f0334);
    ok &= expect_int("corrupt duplicate slot", spec->corrupt_duplicate_slot,
                     4, f0334);
    ok &= expect_int("corrupt duplicate slot value",
                     spec->corrupted_visible_slots[4],
                     spec->corrupt_duplicate_item, f0334);
    ok &= expect_int("corrupt duplicate original still visible",
                     spec->corrupted_visible_slots[1],
                     spec->corrupt_duplicate_item, f0334);
    ok &= expect_int("corrupt replaced item absent from slot 4",
                     spec->corrupted_visible_slots[4] != 0x214 ? 1 : 0,
                     1, f0334);
    ok &= expect_int("corrupt dangling item", spec->corrupt_dangling_next_item,
                     0x217, sentinel);
    ok &= expect_int("corrupt dangling next value",
                     spec->corrupted_next_by_item[7],
                     spec->thing_dangling, sentinel);
    for (i = 0; i < spec->slot_count; ++i) {
        ok &= expect_int("post-corruption visible slot is populated",
                         spec->corrupted_visible_slots[i] != spec->thing_none ?
                         1 : 0,
                         1, f0334);
    }
    return ok;
}

static int test_close_rewrite(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const char *f0334 = spec->anchors.f0334_close_rewrite;
    const char *f0163 = spec->anchors.f0163_link_append;
    const int want_order[5] = { 0x210, 0x211, 0x215, 0x216, 0x217 };
    int ok = 1;
    int i;

    ok &= expect_int("close count", spec->close_count, 5, f0334);
    ok &= expect_int("close container slot", spec->close_container_slot,
                     want_order[0], f0334);
    ok &= expect_int("close cleared visible slots",
                     spec->close_cleared_visible_slots, 1, f0334);
    for (i = 0; i < spec->slot_count; ++i) {
        ok &= expect_int("post-close G0425 slot cleared",
                         spec->post_close_visible_slots[i],
                         spec->thing_none, f0334);
    }
    for (i = 0; i < 5; ++i) {
        ok &= expect_int("close rewrite order",
                         spec->close_order[i], want_order[i], f0163);
    }
    ok &= expect_int("close no duplicate ordinal",
                     spec->close_duplicate_ordinal_count, 0, f0334);
    ok &= expect_int("close no dangling next", spec->close_has_dangling_next,
                     0, f0163);
    ok &= expect_int("close no NONE inside visible window",
                     spec->close_has_none_inside_visible_window, 0, f0334);
    ok &= expect_int("close drops null-next corrupt item",
                     spec->close_dropped_null_next_item, 1, f0334);
    ok &= expect_int("close drops duplicate-shadow item",
                     spec->close_dropped_duplicate_shadow_item, 1, f0334);
    ok &= expect_int("close drops replaced item",
                     spec->close_dropped_replaced_item, 1, f0334);
    ok &= expect_int("post-close B links to F",
                     spec->post_close_next_by_item[1], 0x215, f0163);
    ok &= expect_int("post-close H next is END",
                     spec->post_close_next_by_item[7], spec->thing_end,
                     f0163);
    return ok;
}

static int test_cleanup_and_reopen(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const char *f0164 = spec->anchors.f0164_square_cleanup;
    const char *f0333 = spec->anchors.f0333_open_materialization;
    const int want_order[5] = { 0x210, 0x211, 0x215, 0x216, 0x217 };
    int ok = 1;
    int i;

    ok &= expect_int("cleanup isolates null-next item",
                     spec->cleanup_isolated_null_next_item, 1, f0164);
    ok &= expect_int("cleanup isolates duplicate-shadow item",
                     spec->cleanup_isolated_duplicate_shadow_item, 1, f0164);
    ok &= expect_int("cleanup isolates replaced item",
                     spec->cleanup_isolated_replaced_item, 1, f0164);
    ok &= expect_int("cleanup stopped at NONE or END",
                     spec->cleanup_stopped_at_none_or_end, 1, f0164);
    ok &= expect_int("cleanup C next END",
                     spec->cleanup_next_by_item[2], spec->thing_end, f0164);
    ok &= expect_int("cleanup D next END",
                     spec->cleanup_next_by_item[3], spec->thing_end, f0164);
    ok &= expect_int("cleanup E next END",
                     spec->cleanup_next_by_item[4], spec->thing_end, f0164);
    ok &= expect_int("reopen count", spec->reopen_count, 5, f0333);
    for (i = 0; i < 5; ++i) {
        ok &= expect_int("reopen materialized order",
                         spec->reopen_slots[i], want_order[i], f0333);
    }
    for (i = 5; i < spec->slot_count; ++i) {
        ok &= expect_int("reopen tail NONE",
                         spec->reopen_slots[i], spec->thing_none, f0333);
    }
    ok &= expect_int("reopen tail NONE count",
                     spec->reopen_tail_none_count, 3, f0333);
    ok &= expect_int("reopen no duplicate ordinal",
                     spec->reopen_duplicate_ordinal_count, 0, f0333);
    ok &= expect_int("reopen no dangling next",
                     spec->reopen_has_dangling_next, 0, f0333);
    ok &= expect_int("reopen no END inside visible window",
                     spec->reopen_has_end_inside_visible_window, 0, f0333);
    ok &= expect_int("reopen first slot", spec->reopen_first_slot,
                     want_order[0], f0333);
    ok &= expect_int("reopen last visible slot",
                     spec->reopen_last_visible_slot, want_order[4], f0333);
    return ok;
}

static int test_weight_leader_hand_and_slots(
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    const char *f0140 = spec->anchors.f0140_container_weight;
    const char *f0297 = spec->anchors.f0297_put_leader_hand;
    const char *f0298 = spec->anchors.f0298_remove_leader_hand;
    const char *f0300 = spec->anchors.f0300_clear_c30_slot;
    const char *f0301 = spec->anchors.f0301_write_c30_slot;
    int ok = 1;

    ok &= expect_int("recovered container weight",
                     spec->recovered_container_weight,
                     50 + 2 + 3 + 13 + 17 + 19, f0140);
    ok &= expect_int("weight dropped corrupt contents",
                     spec->initial_container_weight -
                         spec->recovered_container_weight,
                     5 + 7 + 11, f0140);
    ok &= expect_int("leader hand initial thing",
                     spec->leader_hand_initial_thing,
                     DM1_V1_CHEST_LINK_CORRUPTION_CHEST_THING_PC34, f0297);
    ok &= expect_int("leader hand icon identity",
                     spec->leader_hand_icon_before,
                     DM1_V1_CHEST_LINK_CORRUPTION_ICON_CHEST_OPEN_PC34,
                     spec->anchors.f0033_icon_identity);
    ok &= expect_int("leader load after put",
                     spec->leader_load_after_put,
                     spec->leader_load_before_put +
                         spec->recovered_container_weight,
                     f0297);
    ok &= expect_int("leader removed same thing",
                     spec->leader_hand_removed_thing,
                     spec->leader_hand_initial_thing, f0298);
    ok &= expect_int("leader load after remove",
                     spec->leader_load_after_remove,
                     spec->leader_load_before_put, f0298);
    ok &= expect_int("leader final thing restored",
                     spec->leader_hand_final_thing,
                     spec->leader_hand_initial_thing, f0297);
    ok &= expect_int("leader load after restore",
                     spec->leader_load_after_restore,
                     spec->leader_load_after_put, f0297);
    ok &= expect_int("leader hand valid after cycle",
                     spec->leader_hand_valid_after_cycle, 1, f0298);
    ok &= expect_int("C30+ slot index", spec->c30_slot_index,
                     DM1_V1_CHEST_LINK_CORRUPTION_C30_SLOT_PC34 + 3, f0300);
    ok &= expect_int("C30+ clear before", spec->c30_clear_before, 0x215,
                     f0300);
    ok &= expect_int("C30+ clear after", spec->c30_clear_after,
                     spec->thing_none, f0300);
    ok &= expect_int("C30+ write thing", spec->c30_write_thing, 0x217,
                     f0301);
    ok &= expect_int("C30+ write after", spec->c30_write_after,
                     spec->c30_write_thing, f0301);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec =
        dm1_v1_chest_link_corruption_recovery_pc34_compat();
    int ok = 1;

    printf("probe=dm1_v1_chest_link_corruption_recovery_pc34_compat\n");
    ++g_assertions;
    if (!spec) {
        ++g_failures;
        printf("FAIL spec accessor returned NULL\n");
        printf("assertions=%d failures=%d\n", g_assertions, g_failures);
        return 1;
    }
    printf("PASS spec accessor returned non-NULL\n");
    printf("sourceAnchor=%s\n", spec->anchors.f0334_close_rewrite);
    printf("sourceAnchor=%s\n", spec->anchors.f0333_open_materialization);
    printf("sourceAnchor=%s\n", spec->anchors.f0164_square_cleanup);

    ok &= test_anchor_strings(spec);
    ok &= test_initial_state(spec);
    ok &= test_corruption_present(spec);
    ok &= test_close_rewrite(spec);
    ok &= test_cleanup_and_reopen(spec);
    ok &= test_weight_leader_hand_and_slots(spec);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1,
                     spec->anchors.f0334_close_rewrite);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return ok && g_failures == 0 ? 0 : 1;
}
