#include "dm1_v1_chest_pickup_at_capacity_pc34_compat.h"

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

static int expect_nonempty_string(const char *label,
                                  const char *got,
                                  const char *redmcsb_anchor)
{
    ++g_assertions;
    if (!redmcsb_anchor || redmcsb_anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || got[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing text anchor=%s\n", label, redmcsb_anchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsb_anchor);
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

static int test_contract_values(
    const Dm1V1ChestPickupAtCapacityContractPc34Compat *contract)
{
    const char *f0333 = contract->redmcsb_f0333_anchor;
    const char *f0334 = contract->redmcsb_f0334_anchor;
    const char *f0297 = contract->redmcsb_f0297_anchor;
    const char *f0298 = contract->redmcsb_f0298_anchor;
    const char *defs = contract->redmcsb_defs_c08_anchor;
    int ok = 1;

    ok &= expect_int("contract_only", contract->contract_only, 1, f0333);
    ok &= expect_int("chest_visible_capacity",
                     contract->chest_visible_capacity, 8, defs);
    ok &= expect_int("chest_filled_slots_at_start",
                     contract->chest_filled_slots_at_start, 8, f0333);
    ok &= expect_int("leader_hand_thing_at_start nonzero",
                     contract->leader_hand_thing_at_start != 0 ? 1 : 0,
                     1, f0297);
    ok &= expect_int("leader_hand_weight_at_start",
                     contract->leader_hand_weight_at_start, 17, f0297);
    ok &= expect_int("pickup_invoked_with_full_chest",
                     contract->pickup_invoked_with_full_chest, 1, f0298);
    ok &= expect_int("pickup_accepted",
                     contract->pickup_accepted, 0, f0298);
    ok &= expect_int("leader_hand_thing_after_pickup",
                     contract->leader_hand_thing_after_pickup,
                     contract->leader_hand_thing_at_start, f0298);
    ok &= expect_int("leader_hand_weight_after_pickup",
                     contract->leader_hand_weight_after_pickup,
                     contract->leader_hand_weight_at_start, f0297);
    ok &= expect_int("chest_visible_slots_unchanged",
                     contract->chest_visible_slots_unchanged, 1, f0333);
    ok &= expect_int("chest_unchanged_count",
                     contract->chest_unchanged_count, 8, f0333);
    ok &= expect_int("hidden_tail_unchanged",
                     contract->hidden_tail_unchanged, 1, f0333);
    ok &= expect_int("no_recompaction_required",
                     contract->no_recompaction_required, 1, f0334);
    ok &= expect_int("no_error_path_to_f0334",
                     contract->no_error_path_to_f0334, 1, f0334);
    return ok;
}

static int test_anchor_text(
    const Dm1V1ChestPickupAtCapacityContractPc34Compat *contract)
{
    const char *f0333 = contract->redmcsb_f0333_anchor;
    const char *f0334 = contract->redmcsb_f0334_anchor;
    const char *f0297 = contract->redmcsb_f0297_anchor;
    const char *f0298 = contract->redmcsb_f0298_anchor;
    const char *defs = contract->redmcsb_defs_c08_anchor;
    int ok = 1;

    ok &= expect_nonempty_string("redmcsb_f0333_anchor", f0333, f0333);
    ok &= expect_nonempty_string("redmcsb_f0334_anchor", f0334, f0334);
    ok &= expect_nonempty_string("redmcsb_f0297_anchor", f0297, f0297);
    ok &= expect_nonempty_string("redmcsb_f0298_anchor", f0298, f0298);
    ok &= expect_nonempty_string("redmcsb_defs_c08_anchor", defs, defs);

    ok &= expect_contains("F0333 anchor file range", f0333,
                          "CHEST.C F0333:31-67", f0333);
    ok &= expect_contains("F0333 anchor visible slots", f0333,
                          "G0425_aT_ChestSlots[0..7]", f0333);
    ok &= expect_contains("F0333 anchor last writable index", f0333,
                          "slot index 7", f0333);
    ok &= expect_contains("F0334 anchor file range", f0334,
                          "CHEST.C F0334:113-132", f0334);
    ok &= expect_contains("F0334 anchor visible rewrite", f0334,
                          "non-empty visible", f0334);
    ok &= expect_contains("F0297 anchor file range", f0297,
                          "CHAMPION.C F0297:243-268", f0297);
    ok &= expect_contains("F0297 anchor weight", f0297,
                          "adds its object weight", f0297);
    ok &= expect_contains("F0298 anchor file range", f0298,
                          "CHAMPION.C F0298:270-298", f0298);
    ok &= expect_contains("F0298 anchor F0302 range", f0298,
                          "F0302:688-710", f0298);
    ok &= expect_contains("DEFS anchor chest slots", defs,
                          "C30_SLOT_CHEST_1..C37_SLOT_CHEST_8:810-817",
                          defs);
    ok &= expect_contains("DEFS anchor C08/C09", defs,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT/"
                          "C09_SLOT_BOX_INVENTORY_ACTION_HAND:1874-1875",
                          defs);
    ok &= expect_contains("DEFS anchor zones", defs,
                          "C537_ZONE_SLOT_BOX_38_CHEST_1.."
                          "C544_ZONE_SLOT_BOX_45_CHEST_8:3906-3913",
                          defs);
    ok &= expect_contains("DEFS anchor array", defs,
                          "G0425_aT_ChestSlots[8]:5878", defs);
    return ok;
}

static int test_required_phrases(
    const Dm1V1ChestPickupAtCapacityContractPc34Compat *contract)
{
    const char *note = contract->capacity_note;
    const char *summary = contract->source_summary;
    const char *f0333 = contract->redmcsb_f0333_anchor;
    int ok = 1;

    ok &= expect_contains("capacity note contract marker", note,
                          "contract_only=1", f0333);
    ok &= expect_contains("capacity note F0333", note,
                          "CHEST.C F0333:31-67", f0333);
    ok &= expect_contains("capacity note F0334", note,
                          "CHEST.C F0334:113-132",
                          contract->redmcsb_f0334_anchor);
    ok &= expect_contains("capacity note champion", note,
                          "CHAMPION.C F0297/F0298:243-285",
                          contract->redmcsb_f0298_anchor);
    ok &= expect_contains("capacity note visible capacity", note,
                          "visible capacity=8", f0333);
    ok &= expect_contains("capacity note rejected", note,
                          "pickup rejected", contract->redmcsb_f0298_anchor);
    ok &= expect_contains("capacity note leader hand", note,
                          "leader hand unchanged",
                          contract->redmcsb_f0297_anchor);
    ok &= expect_contains("capacity note chest slots", note,
                          "chest slots unchanged", f0333);
    ok &= expect_contains("capacity note no rewrite", note,
                          "no F0334 rewrite",
                          contract->redmcsb_f0334_anchor);

    ok &= expect_contains("source summary contract marker", summary,
                          "contract_only=1", f0333);
    ok &= expect_contains("source summary F0333", summary,
                          "CHEST.C F0333:31-67", f0333);
    ok &= expect_contains("source summary F0334", summary,
                          "CHEST.C F0334:113-132",
                          contract->redmcsb_f0334_anchor);
    ok &= expect_contains("source summary champion", summary,
                          "CHAMPION.C F0297/F0298:243-285",
                          contract->redmcsb_f0298_anchor);
    ok &= expect_contains("source summary visible capacity", summary,
                          "visible capacity=8", f0333);
    ok &= expect_contains("source summary rejected", summary,
                          "pickup rejected", contract->redmcsb_f0298_anchor);
    ok &= expect_contains("source summary leader hand", summary,
                          "leader hand unchanged",
                          contract->redmcsb_f0297_anchor);
    ok &= expect_contains("source summary chest slots", summary,
                          "chest slots unchanged", f0333);
    ok &= expect_contains("source summary no rewrite", summary,
                          "no F0334 rewrite",
                          contract->redmcsb_f0334_anchor);
    return ok;
}

int main(void)
{
    const Dm1V1ChestPickupAtCapacityContractPc34Compat *contract =
        dm1_v1_chest_pickup_at_capacity_contract_pc34_compat();
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_at_capacity_pc34_compat\n");
    if (!contract) {
        ++g_assertions;
        ++g_failures;
        printf("FAIL contract missing\n");
        printf("assertions=%d\n", g_assertions);
        return 1;
    }

    printf("sourceAnchor=%s\n", contract->redmcsb_f0333_anchor);
    ok &= test_contract_values(contract);
    ok &= test_anchor_text(contract);
    ok &= test_required_phrases(contract);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 45 ? 1 : 0, 1,
                     contract->redmcsb_f0333_anchor);

    printf("assertions=%d\n", g_assertions);
    printf("failures=%d\n", g_failures);
    printf("chestPickupAtCapacityInvariantOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    return ok && g_failures == 0 ? 0 : 1;
}
