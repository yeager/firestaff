#include "dm1_v1_inventory_hand_belt_round_trip_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_passes;

static int expect_int(const char* label, int got, int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int expect_contains(const char* label, const char* got,
                           const char* want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing '%s' anchor=%s\n", label,
               want ? want : "(null)", anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int test_spec(void)
{
    const DM1_V1_InventoryHandBeltRoundTripSpecPc34* spec =
        dm1_v1_inventory_hand_belt_round_trip_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_hand_belt_round_trip_evidence_pc34();
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* dataMasks = "ReDMCSB DATA.C lines 1049-1079";
    const char* dataStorage = "ReDMCSB DATA.C lines 1128-1142";
    int ok = 1;

    ok &= expect_int("spec exists", spec != 0, 1, f0302);
    ok &= expect_int("spec contract only", spec->contractOnly, 1, f0302);
    ok &= expect_int("ready hand pc34", spec->readyHandPc34Slot,
                     DM1_PC34_SLOT_READY_HAND, dataMasks);
    ok &= expect_int("action hand pc34", spec->actionHandPc34Slot,
                     DM1_PC34_SLOT_ACTION_HAND, dataMasks);
    ok &= expect_int("belt C19 pc34", spec->beltC19Pc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE2_7, dataStorage);
    ok &= expect_int("belt C20 pc34", spec->beltC20Pc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE2_8, dataStorage);
    ok &= expect_int("belt C21 pc34", spec->beltC21Pc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE2_9, dataStorage);
    ok &= expect_int("belt C22 pc34", spec->beltC22Pc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_2, dataStorage);
    ok &= expect_contains("defs anchor", spec->defsAnchor,
                          "1874-1878", "ReDMCSB DEFS.H lines 1874-1878");
    ok &= expect_contains("swap anchor", spec->championSwapAnchor,
                          "F0302", f0302);
    ok &= expect_contains("hand anchor", spec->championHandAnchor,
                          "F0297/F0298",
                          "ReDMCSB CHAMPION.C lines 243-298");
    ok &= expect_contains("mask anchor", spec->dataMaskAnchor,
                          "1049-1079", dataMasks);
    ok &= expect_contains("storage anchor", spec->dataStorageAnchor,
                          "1128-1142", dataStorage);
    ok &= expect_contains("scope contract", spec->scope,
                          "contract_only=1", f0302);
    ok &= expect_contains("scope no real asset", spec->scope,
                          "no real-asset", f0302);
    ok &= expect_contains("evidence f0302", evidence, "F0302", f0302);
    ok &= expect_contains("evidence c19 c22", evidence, "C19-C22",
                          dataStorage);
    return ok;
}

static int test_slot_mapping(
    const char* name,
    const DM1_V1_InventoryHandBeltRoundTripSlotPc34* slot,
    int wantPc34,
    int wantStorage,
    int wantMask,
    const char* anchor)
{
    char label[96];
    int ok = 1;

    snprintf(label, sizeof(label), "%s pc34", name);
    ok &= expect_int(label, slot->pc34Slot, wantPc34, anchor);
    snprintf(label, sizeof(label), "%s storage", name);
    ok &= expect_int(label, slot->storageSlot, wantStorage, anchor);
    snprintf(label, sizeof(label), "%s mask", name);
    ok &= expect_int(label, slot->slotMask, wantMask, anchor);
    return ok;
}

static int test_probe(void)
{
    DM1_V1_InventoryHandBeltRoundTripProbePc34 probe;
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300/F0301 lines 511-615";
    const char* dataMasks = "ReDMCSB DATA.C lines 1049-1079";
    const char* dataStorage = "ReDMCSB DATA.C lines 1128-1142";
    int ok = 1;

    ok &= expect_int("probe builds",
                     dm1_v1_inventory_hand_belt_round_trip_probe_pc34(
                         &probe),
                     1, f0302);
    ok &= expect_int("probe contract", probe.contractOnly, 1, f0302);
    ok &= expect_int("assertion budget", probe.assertionBudget, 80, f0302);

    ok &= test_slot_mapping("ready", &probe.readySlot,
                            DM1_PC34_SLOT_READY_HAND,
                            DM1_SLOT_HAND_RIGHT,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataMasks);
    ok &= test_slot_mapping("action", &probe.actionSlot,
                            DM1_PC34_SLOT_ACTION_HAND,
                            DM1_SLOT_HAND_LEFT,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataMasks);
    ok &= test_slot_mapping("belt C19", &probe.beltC19Slot,
                            DM1_PC34_SLOT_BACKPACK_LINE2_7,
                            DM1_SLOT_BACKPACK7,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataStorage);
    ok &= test_slot_mapping("belt C20", &probe.beltC20Slot,
                            DM1_PC34_SLOT_BACKPACK_LINE2_8,
                            DM1_SLOT_BACKPACK8,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataStorage);
    ok &= test_slot_mapping("belt C21", &probe.beltC21Slot,
                            DM1_PC34_SLOT_BACKPACK_LINE2_9,
                            DM1_SLOT_BACKPACK9,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataStorage);
    ok &= test_slot_mapping("belt C22", &probe.beltC22Slot,
                            DM1_PC34_SLOT_BACKPACK_LINE1_2,
                            DM1_SLOT_BACKPACK10,
                            DM1_PC34_ALLOWED_ANY_SLOT, dataStorage);

    ok &= expect_int("initial load", probe.initialLoad, 32, f0300);
    ok &= expect_int("leader before ready click",
                     probe.leaderHandBeforeReadyClick,
                     DM1_V1_IHBRT_LEADER_HAND_ITEM, f0297);
    ok &= expect_int("ready click result", probe.readyClickResult, 1,
                     f0302);
    ok &= expect_int("ready receives leader object",
                     probe.readyAfterReadyClick,
                     DM1_V1_IHBRT_LEADER_HAND_ITEM, f0302);
    ok &= expect_int("old ready moves to leader hand",
                     probe.leaderAfterReadyClick,
                     DM1_V1_IHBRT_READY_ITEM, f0297);
    ok &= expect_int("load after ready click", probe.loadAfterReadyClick,
                     38, f0300);

    ok &= expect_int("belt C19 click result", probe.beltC19ClickResult, 1,
                     f0302);
    ok &= expect_int("belt C19 receives old ready",
                     probe.beltC19AfterClick,
                     DM1_V1_IHBRT_READY_ITEM, f0302);
    ok &= expect_int("old belt C19 moves to leader",
                     probe.leaderAfterBeltC19Click,
                     DM1_V1_IHBRT_BELT_C19_ITEM, f0297);
    ok &= expect_int("load after belt C19 click",
                     probe.loadAfterBeltC19Click, 46, f0300);

    ok &= expect_int("action click result", probe.actionClickResult, 1,
                     f0302);
    ok &= expect_int("action receives old C19",
                     probe.actionAfterClick,
                     DM1_V1_IHBRT_BELT_C19_ITEM, f0302);
    ok &= expect_int("old action moves to leader",
                     probe.leaderAfterActionClick,
                     DM1_V1_IHBRT_ACTION_ITEM, f0297);
    ok &= expect_int("load after action click", probe.loadAfterActionClick,
                     36, f0300);

    ok &= expect_int("belt C20 click result", probe.beltC20ClickResult, 1,
                     f0302);
    ok &= expect_int("belt C20 receives old action",
                     probe.beltC20AfterClick,
                     DM1_V1_IHBRT_ACTION_ITEM, f0302);
    ok &= expect_int("old belt C20 moves to leader",
                     probe.leaderAfterBeltC20Click,
                     DM1_V1_IHBRT_BELT_C20_ITEM, f0297);
    ok &= expect_int("load after belt C20 click",
                     probe.loadAfterBeltC20Click, 44, f0300);

    ok &= expect_int("belt C21 empty click result",
                     probe.beltC21EmptyClickResult, 1, f0302);
    ok &= expect_int("belt C21 receives old C20",
                     probe.beltC21AfterEmptyClick,
                     DM1_V1_IHBRT_BELT_C20_ITEM, f0302);
    ok &= expect_int("leader clears after empty belt insert",
                     probe.leaderAfterBeltC21EmptyClick, 0, f0297);
    ok &= expect_int("load after empty belt insert",
                     probe.loadAfterBeltC21EmptyClick, 49, f0300);

    ok &= expect_int("ready pickup result", probe.readyPickupResult, 1,
                     f0302);
    ok &= expect_int("ready empty after pickup", probe.readyAfterPickup, 0,
                     f0300);
    ok &= expect_int("leader gets ready item",
                     probe.leaderAfterReadyPickup,
                     DM1_V1_IHBRT_LEADER_HAND_ITEM, f0297);
    ok &= expect_int("load after ready pickup",
                     probe.loadAfterReadyPickup, 32, f0300);

    ok &= expect_int("belt C22 reinsert result",
                     probe.beltC22ReinsertResult, 1, f0302);
    ok &= expect_int("belt C22 receives ready item",
                     probe.beltC22AfterReinsert,
                     DM1_V1_IHBRT_LEADER_HAND_ITEM, f0302);
    ok &= expect_int("leader clears after C22 reinsert",
                     probe.leaderAfterBeltC22Reinsert, 0, f0297);
    ok &= expect_int("load after C22 reinsert",
                     probe.loadAfterBeltC22Reinsert, 49, f0300);

    ok &= expect_int("action pickup result", probe.actionPickupResult, 1,
                     f0302);
    ok &= expect_int("action empty after pickup", probe.actionAfterPickup, 0,
                     f0300);
    ok &= expect_int("leader receives action occupant",
                     probe.leaderAfterActionPickup,
                     DM1_V1_IHBRT_BELT_C19_ITEM, f0297);
    ok &= expect_int("load after action pickup",
                     probe.loadAfterActionPickup, 46, f0300);

    ok &= expect_int("pouch reject result", probe.pouchRejectResult, 0,
                     "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= expect_int("pouch occupant preserved", probe.pouchAfterReject,
                     DM1_V1_IHBRT_POUCH_OCCUPANT_ITEM,
                     "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= expect_int("leader preserved after pouch reject",
                     probe.leaderAfterPouchReject,
                     DM1_V1_IHBRT_HEAD_ONLY_ITEM,
                     "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= expect_int("load preserved after pouch reject",
                     probe.loadAfterPouchReject, 53,
                     "ReDMCSB CHAMPION.C F0302 lines 697-699");
    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_spec();
    ok &= test_probe();
    if (!ok) {
        printf("FAIL dm1_v1_inventory_hand_belt_round_trip_pc34_compat "
               "%d/%d assertions\n", g_passes, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_inventory_hand_belt_round_trip_pc34_compat "
           "%d/%d assertions\n", g_passes, g_assertions);
    return 0;
}
