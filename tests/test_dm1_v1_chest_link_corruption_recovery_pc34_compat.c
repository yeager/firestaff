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

static int expect_case_order(
    const char *label,
    const DM1_V1_ChestLinkCorruptionRecoveryCasePc34 *case_contract,
    const int *want_close_order,
    int want_count,
    const char *f0334,
    const char *f0333)
{
    int ok = 1;
    int i;

    ok &= expect_int(label, case_contract ? 1 : 0, 1, f0334);
    if (!case_contract) {
        return 0;
    }
    ok &= expect_int("close count", case_contract->close_count, want_count,
                     f0334);
    ok &= expect_int("reopen count", case_contract->reopen_count, want_count,
                     f0333);
    ok &= expect_int("chain contains no NONE",
                     case_contract->chain_contains_none, 0, f0334);
    ok &= expect_int("chain terminates with END",
                     case_contract->chain_terminator,
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST, f0333);
    ok &= expect_int("last valid Next points to END",
                     case_contract->last_valid_next_after_close,
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST, f0334);
    ok &= expect_int("Container->Slot reset to END before head write",
                     case_contract->container_slot_after_end_reset,
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST, f0334);
    ok &= expect_int("Container->Slot head is first valid item",
                     case_contract->container_slot_after_head_write,
                     want_close_order[0], f0334);
    ok &= expect_int("last valid item recorded",
                     case_contract->last_valid_thing,
                     want_close_order[want_count - 1], f0334);
    for (i = 0; i < want_count; ++i) {
        ok &= expect_int("close preserves visible order",
                         case_contract->close_order[i],
                         want_close_order[i], f0334);
        ok &= expect_int("reopen materializes visible order",
                         case_contract->reopen_slots[i],
                         want_close_order[i], f0333);
    }
    for (i = want_count; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT; ++i) {
        ok &= expect_int("reopen fills trailing visible slots with NONE",
                         case_contract->reopen_slots[i],
                         DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, f0333);
    }
    return ok;
}

static int test_contract_values(
    const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 *contract)
{
    const char *f0334 = contract->redmcsb_f0334_anchor;
    const char *f0333 = contract->redmcsb_f0333_anchor;
    const char *sentinel = contract->sentinel_anchor;
    const int mid_order[6] = { 0x210, 0x220, 0x230, 0x240, 0x250, 0x260 };
    const int leading_order[3] = { 0x310, 0x320, 0x330 };
    int ok = 1;

    ok &= expect_int("contract_only", contract->contract_only, 1, f0334);
    ok &= expect_int("visible slot count", contract->visible_slot_count, 8,
                     f0334);
    ok &= expect_int("C0xFFFF_THING_NONE value", contract->thing_none,
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, sentinel);
    ok &= expect_int("C0xFFFE_THING_ENDOFLIST value",
                     contract->thing_end_of_list,
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST, sentinel);
    ok &= expect_int("F0334 close loop bound", contract->close_loop_bound, 8,
                     f0334);
    ok &= expect_int("F0334 skips NONE slots",
                     contract->close_loop_skips_none, 1, f0334);
    ok &= expect_int("F0333 walks rewritten links",
                     contract->reopen_loop_walks_rewritten_links, 1, f0333);
    ok &= expect_int("F0333 stops at END",
                     contract->reopen_loop_stops_at_end, 1, f0333);
    ok &= expect_int("F0333 fills empty tail with NONE",
                     contract->open_empty_tail_fills_none, 1, f0333);

    ok &= expect_case_order("mid-array NONE case present",
                            &contract->mid_array_none, mid_order, 6,
                            f0334, f0333);
    ok &= expect_int("mid-array input slot two is NONE",
                     contract->mid_array_none.input_slots[2],
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, sentinel);
    ok &= expect_int("mid-array input slot five is NONE",
                     contract->mid_array_none.input_slots[5],
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, sentinel);
    ok &= expect_int("mid-array close avoids slot-count regression",
                     contract->mid_array_none.close_count >
                         contract->mid_array_none.pre_sentinel_count_if_regressed ?
                         1 : 0,
                     1, f0334);

    ok &= expect_case_order("leading NONE case present",
                            &contract->leading_none, leading_order, 3,
                            f0334, f0333);
    ok &= expect_int("leading input slot zero is NONE",
                     contract->leading_none.input_slots[0],
                     DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, sentinel);
    ok &= expect_int("leading NONE does not short-circuit close",
                     contract->leading_none.close_count, 3, f0334);
    ok &= expect_int("leading NONE does not short-circuit reopen",
                     contract->leading_none.reopen_slots[0], leading_order[0],
                     f0333);
    return ok;
}

static int test_anchor_text(
    const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 *contract)
{
    const char *f0334 = contract->redmcsb_f0334_anchor;
    const char *f0333 = contract->redmcsb_f0333_anchor;
    const char *sentinel = contract->sentinel_anchor;
    int ok = 1;

    ok &= expect_contains("F0334 anchor range", f0334,
                          "CHEST.C F0334:117-132", f0334);
    ok &= expect_contains("F0334 anchor Container slot", f0334,
                          "Container->Slot", f0334);
    ok &= expect_contains("F0334 anchor G0425", f0334,
                          "G0425_aT_ChestSlots", f0334);
    ok &= expect_contains("F0334 anchor skips NONE", f0334,
                          "C0xFFFF_THING_NONE", f0334);
    ok &= expect_contains("F0333 anchor range", f0333,
                          "CHEST.C F0333:53-67", f0333);
    ok &= expect_contains("F0333 anchor starts at Container slot", f0333,
                          "Container->Slot", f0333);
    ok &= expect_contains("F0333 anchor stops at END", f0333,
                          "C0xFFFE_THING_ENDOFLIST", f0333);
    ok &= expect_contains("sentinel anchor NONE", sentinel,
                          "C0xFFFF_THING_NONE", sentinel);
    ok &= expect_contains("sentinel anchor END", sentinel,
                          "C0xFFFE_THING_ENDOFLIST", sentinel);
    ok &= expect_contains("contract note marker", contract->contract_note,
                          "contract_only=1", f0334);
    ok &= expect_contains("contract note mid sentinel", contract->contract_note,
                          "mid-array C0xFFFF_THING_NONE", f0334);
    ok &= expect_contains("contract note leading sentinel",
                          contract->contract_note,
                          "leading C0xFFFF_THING_NONE", f0334);
    ok &= expect_contains("source summary marker", contract->source_summary,
                          "contract_only=1", f0334);
    ok &= expect_contains("source summary close range", contract->source_summary,
                          "CHEST.C F0334:117-132", f0334);
    ok &= expect_contains("source summary open range", contract->source_summary,
                          "CHEST.C F0333:53-67", f0333);
    ok &= expect_contains("source summary last END", contract->source_summary,
                          "last valid Next remains END", f0334);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 *contract =
        dm1_v1_chest_link_corruption_recovery_contract_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_link_corruption_recovery_pc34_compat\n");
    ++g_assertions;
    if (!contract) {
        ++g_failures;
        printf("FAIL contract accessor returned NULL\n");
        printf("assertions=%d failures=%d\n", g_assertions, g_failures);
        return 1;
    }
    printf("PASS contract accessor returned non-NULL\n");
    printf("sourceAnchor=%s\n", contract->redmcsb_f0334_anchor);

    ok &= test_contract_values(contract);
    ok &= test_anchor_text(contract);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1,
                     contract->redmcsb_f0334_anchor);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return ok && g_failures == 0 ? 0 : 1;
}
