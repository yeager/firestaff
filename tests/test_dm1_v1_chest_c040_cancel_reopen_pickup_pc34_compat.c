#include "firestaff/dm1/v1/chest/c040_cancel_reopen_pickup_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_u32_nonzero(uint32_t actual, const char *message,
                              const char *anchor)
{
    ++g_assertions;
    if (actual == 0u) {
        ++g_failures;
        printf("FAIL %s actual=0x%08x [%s]\n", message, actual,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message,
               needle ? needle : "(null)", anchor ? anchor : "(null)");
    }
}

static void test_source_evidence(void)
{
    const Dm1V1ChestC040CancelReopenPickupEvidencePc34 *e =
        dm1_v1_chest_c040_cancel_reopen_pickup_evidence_pc34();
    const char *source =
        dm1_v1_chest_c040_cancel_reopen_pickup_source_evidence_pc34();

    check_true(e != NULL, "evidence exists", "source-lock");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "chest open anchor", e->chestOpenAnchor);
    check_contains(e->chestOpenAnchor, "G0425",
                   "chest open materializes G0425", e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:113-132",
                   "chest close anchor", e->chestCloseAnchor);
    check_contains(e->panelToggleAnchor, "PANEL.C F0355:2299-2318",
                   "panel toggle anchor", e->panelToggleAnchor);
    check_contains(e->reviveCancelAnchor, "REVIVE.C F0282:744-783",
                   "revive cancel anchor", e->reviveCancelAnchor);
    check_contains(e->commandQueueAnchor, "COMMAND.C F0380:2045-2178",
                   "command queue anchor", e->commandQueueAnchor);
    check_contains(e->championSlotAnchor, "CHAMPION.C F0302:688-710",
                   "champion slot anchor", e->championSlotAnchor);
    check_contains(e->defsAnchor, "G0299", "defs G0299", e->defsAnchor);
    check_contains(e->defsAnchor, "G0426", "defs G0426", e->defsAnchor);
    check_contains(e->contractScope, "contract-only",
                   "contract-only marker", e->contractScope);
    check_contains(e->nonOverlap, "mid-F0355/F0334",
                   "non-overlap names mid cancel", e->nonOverlap);
    check_contains(e->nonOverlap, "pickup waits for F0333",
                   "non-overlap names queued reopen pickup", e->nonOverlap);
    check_contains(e->nonOverlap, "c040 panel browse pickup rotate race",
                   "non-overlap excludes rotation sibling", e->nonOverlap);
    check_contains(e->nonOverlap, "chest close pending panel",
                   "non-overlap excludes pending panel sibling", e->nonOverlap);
    check_contains(e->nonOverlap, "pickup during resurrect pending non-leader",
                   "non-overlap excludes resurrect pickup sibling",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "save/load",
                   "non-overlap excludes save load", e->nonOverlap);
    check_contains(e->nonOverlap, "teleporter survival",
                   "non-overlap excludes teleporter", e->nonOverlap);

    check_contains(source, "CHEST.C F0333:30-67",
                   "source includes F0333", source);
    check_contains(source, "CHEST.C F0334:113-132",
                   "source includes F0334", source);
    check_contains(source, "PANEL.C F0355:2299-2318",
                   "source includes F0355", source);
    check_contains(source, "REVIVE.C F0282:744-783",
                   "source includes F0282", source);
    check_contains(source, "COMMAND.C F0380:2045-2178",
                   "source includes F0380", source);
    check_contains(source, "CHAMPION.C F0302:688-710",
                   "source includes F0302", source);
}

static void test_initial_fixture(void)
{
    Dm1V1ChestC040CancelReopenPickupStatePc34 state;
    int i;

    dm1_v1_chest_c040_cancel_reopen_pickup_init_pc34(&state);

    check_int_eq(state.panelContent, 568, "fixture starts on C040 panel",
                 "COMMAND.C F0378:1985-1990");
    check_int_eq(state.c040PanelOpen, 1, "fixture C040 is open",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.g0299CandidateOrdinal, 3, "fixture G0299 candidate",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.inventoryChampionOrdinal, 3,
                 "fixture inventory owner is candidate",
                 "PANEL.C F0355:2299-2318");
    check_int_eq(state.partyChampionCount, 3, "fixture party count",
                 "REVIVE.C F0282:744-783");
    check_int_eq(state.leaderIndex, 0, "fixture leader index",
                 "COMMAND.C F0380:2174-2178");
    check_int_eq(state.leaderHandThing,
                 DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
                 "fixture leader hand empty", "CHAMPION.C F0302:694-710");
    check_int_eq(state.openChestThing, 0x7040, "fixture G0426 open chest",
                 "CHEST.C F0333:43-65");
    check_int_eq(state.containerHeadThing, 0x3100,
                 "fixture container head linked", "CHEST.C F0334:123-129");
    check_int_eq(state.requestedPickupSlot, 2,
                 "fixture pickup targets a visible G0425 slot",
                 "CHAMPION.C F0302:688-690");
    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34; ++i) {
        check_int_eq(state.chestSlots[i], 0x3100 + i,
                     "fixture visible chest slot populated",
                     "CHEST.C F0333:53-67");
        check_int_eq(state.containerSlots[i], 0x3100 + i,
                     "fixture backing container slot populated",
                     "CHEST.C F0334:117-132");
    }
    check_u32_nonzero(state.chestHash, "fixture chest hash",
                      "CHEST.C F0333:30-67");
    check_u32_nonzero(state.stateHash, "fixture state hash",
                      "COMMAND.C F0380:2045-2178");
}

static void test_c040_cancel_reopen_pickup_contract(void)
{
    Dm1V1ChestC040CancelReopenPickupStatePc34 state;
    Dm1V1ChestC040CancelReopenPickupResultPc34 result;
    int ok;

    dm1_v1_chest_c040_cancel_reopen_pickup_init_pc34(&state);
    ok = dm1_v1_chest_c040_cancel_reopen_pickup_run_pc34(&state, &result);

    check_int_eq(ok, 1, "contract run accepted",
                 "CHEST.C F0333:30-67 + REVIVE.C F0282:744-783");
    check_int_eq(result.accepted, 1, "result accepted",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(result.initialPanelContent, 568,
                 "initial panel is C040 resurrect candidate",
                 "COMMAND.C F0378:1985-1990");
    check_int_eq(result.finalPanelContent, 569,
                 "final panel is reopened chest",
                 "CHEST.C F0333:27-30");
    check_int_eq(result.c040OpenBefore, 1, "C040 open before cancel",
                 "REVIVE.C F0280:124-132");
    check_int_eq(result.c040OpenAfter, 0, "C040 closed after cancel",
                 "REVIVE.C F0282:744-783");
    check_int_eq(result.g0299Before, 3, "G0299 set before cancel",
                 "REVIVE.C F0280:124-132");
    check_int_eq(result.g0299After, 0, "G0299 clear after F0355",
                 "REVIVE.C F0282:744-783");
    check_int_eq(result.inventoryOrdinalBefore, 3,
                 "inventory owner set before F0355",
                 "PANEL.C F0355:2299-2318");
    check_int_eq(result.inventoryOrdinalAfter, 0,
                 "inventory owner cleared by F0355",
                 "PANEL.C F0355:2314-2318");
    check_int_eq(result.partyCountBefore, 3, "party count before cancel",
                 "REVIVE.C F0282:744-757");
    check_int_eq(result.partyCountAfter, 2, "party count decremented",
                 "REVIVE.C F0282:751-757");
    check_int_eq(result.leaderIndexBefore, 0, "leader before",
                 "COMMAND.C F0380:2174-2178");
    check_int_eq(result.leaderIndexAfter, 0, "leader unchanged",
                 "COMMAND.C F0380:2174-2178");
    check_int_eq(result.leaderHandBefore,
                 DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
                 "leader hand empty before pickup",
                 "CHAMPION.C F0302:688-695");
    check_int_eq(result.leaderHandAfter, result.slotThingBefore,
                 "leader hand receives rematerialized slot item",
                 "CHAMPION.C F0302:700-710");
    check_int_eq(result.openChestBefore, 0x7040, "open chest before cancel",
                 "CHEST.C F0333:43-65");
    check_int_eq(result.openChestAfterCancelClose,
                 DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
                 "F0355 cancel close clears G0426",
                 "PANEL.C F0355:2314-2318");
    check_int_eq(result.openChestAfterReopen, 0x7040,
                 "queued reopen restores G0426",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.openChestAfterPickup, 0x7040,
                 "pickup leaves chest open",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(result.slotThingBefore, 0x3102,
                 "pickup target before cancel",
                 "CHEST.C F0333:64-65");
    check_int_eq(result.slotThingAfterCancelClose,
                 DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
                 "F0334 clears visible G0425 target",
                 "CHEST.C F0334:117-132");
    check_int_eq(result.slotThingAfterReopen, 0x3102,
                 "F0333 rematerializes target before pickup",
                 "CHEST.C F0333:53-67");
    check_int_eq(result.slotThingAfterPickup,
                 DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
                 "F0302 removes picked slot",
                 "CHAMPION.C F0302:704-710");
    check_int_eq(result.containerHeadAfterClose, 0x3100,
                 "F0334 relinks first visible item",
                 "CHEST.C F0334:123-129");
    check_int_eq(result.pickedThing, 0x3102, "picked thing id",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(result.requestedPickupSlot, 2, "requested pickup slot",
                 "CHAMPION.C F0302:688-690");

    check_int_eq(result.f0282CancelCount, 1, "one F0282 cancel",
                 "REVIVE.C F0282:744-783");
    check_int_eq(result.f0355ToggleCount, 1, "one F0355 close",
                 "PANEL.C F0355:2299-2318");
    check_int_eq(result.f0334CloseCount, 1, "one F0334 close",
                 "CHEST.C F0334:113-132");
    check_int_eq(result.f0333OpenCount, 1, "one F0333 reopen",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.f0380DispatchCount, 1, "one F0380 slot dispatch",
                 "COMMAND.C F0380:2045-2178");
    check_int_eq(result.f0302PickupCount, 1, "one F0302 pickup",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(result.unsafePickupRejectCount, 1,
                 "unsafe mid-cancel pickup rejected once",
                 "REVIVE.C F0282:744-783");

    check_int_eq(result.cancelClosedChestBeforeCandidateClear, 1,
                 "cancel closes chest before candidate clear",
                 "REVIVE.C F0282:744-757");
    check_int_eq(result.candidateClearedAfterF0355, 1,
                 "candidate clear follows F0355",
                 "REVIVE.C F0282:744-757");
    check_int_eq(result.pickupRejectedWhileCancelMidF0355, 1,
                 "pickup rejected while F0355 is mid-cancel",
                 "PANEL.C F0355:2314-2318");
    check_int_eq(result.reopenRematerializedG0425, 1,
                 "reopen rematerialized G0425",
                 "CHEST.C F0333:53-67");
    check_int_eq(result.pickupWaitedForReopen, 1,
                 "pickup waited for queued reopen",
                 "COMMAND.C F0380:2045-2178");
    check_int_eq(result.pickupRanAfterCandidateClear, 1,
                 "pickup ran after G0299 clear",
                 "CHAMPION.C F0302:677-690");
    check_int_eq(result.noDuplicateClose, 1, "no duplicate close",
                 "CHEST.C F0334:113-132");
    check_int_eq(result.noDuplicateReopen, 1, "no duplicate reopen",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.noRotationPath, 1, "rotation path not exercised",
                 "COMMAND.C F0380:2150-2152");
    check_int_eq(result.noSaveLoadTeleporterPath, 1,
                 "save/load and teleporter paths not exercised",
                 "Non-overlap");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "source-lock");

    check_int_eq(result.trace[0], 100, "trace init",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.trace[1], 101, "trace queue",
                 "COMMAND.C F0380:2045-2178");
    check_int_eq(result.trace[2], 102, "trace cancel enter",
                 "REVIVE.C F0282:744-783");
    check_int_eq(result.trace[3], 103, "trace close",
                 "CHEST.C F0334:113-132");
    check_int_eq(result.trace[4], 104, "trace unsafe reject",
                 "PANEL.C F0355:2314-2318");
    check_int_eq(result.trace[5], 105, "trace candidate clear",
                 "REVIVE.C F0282:747-757");
    check_int_eq(result.trace[6], 106, "trace reopen",
                 "CHEST.C F0333:30-67");
    check_int_eq(result.trace[7], 107, "trace pickup",
                 "CHAMPION.C F0302:688-710");

    check_u32_nonzero(result.chestHashBefore, "hash before",
                      "CHEST.C F0333:30-67");
    check_u32_nonzero(result.chestHashAfterClose, "hash after close",
                      "CHEST.C F0334:113-132");
    check_u32_nonzero(result.chestHashAfterReopen, "hash after reopen",
                      "CHEST.C F0333:30-67");
    check_u32_nonzero(result.chestHashAfterPickup, "hash after pickup",
                      "CHAMPION.C F0302:688-710");
    check_u32_nonzero(result.deterministicHash, "deterministic hash",
                      "COMMAND.C F0380:2045-2178");
}

int main(void)
{
    printf("probe=dm1_v1_chest_c040_cancel_reopen_pickup_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_c040_cancel_reopen_pickup_source_evidence_pc34());

    test_source_evidence();
    test_initial_fixture();
    test_c040_cancel_reopen_pickup_contract();

    printf("assertionCount=%d\n", g_assertions);
    if (g_failures != 0) {
        printf("dm1_v1_chest_c040_cancel_reopen_pickup_pc34_compat failed: failures=%d\n",
               g_failures);
        return 1;
    }
    printf("dm1_v1_chest_c040_cancel_reopen_pickup_pc34_compat passed: assertions=%d\n",
           g_assertions);
    return 0;
}
