/* ReDMCSB source-lock evidence:
 * COMMAND.C F0380:2174-2183 distinguishes slot-box dispatch from the
 * !G0299 inventory-toggle guard.
 * CHAMPION.C F0302:688-710 owns C30+ chest slot leader-hand swaps.
 * PANEL.C F0355:2314-2318 calls F0334 during inventory close.
 * CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots.
 * REVIVE.C F0282:744-757 C162 cancel invokes F0355 before clearing G0299.
 */
#include "dm1_v1_mirror_candidate_chest_close_pending_panel_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateChestClosePendingPanel_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "COMMAND.C F0380:2174-2183");
    CHECK_REDMCSB(strstr(e->commandInventoryGuardAnchor, "2180-2183") != NULL,
                  "evidence cites the C011 !G0299 guard",
                  e->commandInventoryGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandSlotDispatchAnchor, "2174-2178") != NULL,
                  "evidence cites slot-box dispatch outside the C011 guard",
                  e->commandSlotDispatchAnchor);
    CHECK_REDMCSB(strstr(e->championSlotBoxAnchor, "F0302:688-710") != NULL,
                  "evidence cites F0302 chest-slot swap range",
                  e->championSlotBoxAnchor);
    CHECK_REDMCSB(strstr(e->panelToggleCloseAnchor, "F0355:2314-2318") != NULL,
                  "evidence cites F0355 inventory close to F0334",
                  e->panelToggleCloseAnchor);
    CHECK_REDMCSB(strstr(e->chestCloseAnchor, "F0334:113-132") != NULL,
                  "evidence cites F0334 close/relink range",
                  e->chestCloseAnchor);
    CHECK_REDMCSB(strstr(e->reviveCancelAnchor, "F0282:744-757") != NULL,
                  "evidence cites C162 cancel order",
                  e->reviveCancelAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence marks the gate as contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "open G0426/G0425 chest") != NULL &&
                      strstr(e->nonOverlapNote, "portrait clicks") != NULL,
                  "evidence records non-overlap with sibling gates",
                  e->nonOverlapNote);
}

static void test_fixture_starts_with_c040_and_open_chest(void)
{
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat state;

    DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_M568_C040_PC34_COMPAT,
                  "fixture starts with C040 panel content",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "fixture starts with candidate panel open",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "fixture publishes G0299 candidate ordinal",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 2u,
                  "fixture selects the candidate inventory owner",
                  "PANEL.C F0355:2314-2318");
    CHECK_REDMCSB(state.partyChampionCount == 2u,
                  "fixture includes appended candidate in party count",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "fixture has a leader for slot-box dispatch",
                  "COMMAND.C F0380:2174-2178");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_LEADER_HAND_PC34_COMPAT,
                  "fixture leader hand starts occupied for F0302 swap",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(state.openChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_OPEN_CHEST_PC34_COMPAT,
                  "fixture has G0426 open chest while C040 is pending",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(state.chestSlots[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT0_THING_PC34_COMPAT,
                  "fixture has visible chest slot 0 populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT,
                  "fixture keeps a mid-array empty chest slot",
                  "CHEST.C F0334:119-132");
    CHECK_REDMCSB(state.chestSlots[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT2_THING_PC34_COMPAT,
                  "fixture has visible chest slot 2 populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[3] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT3_THING_PC34_COMPAT,
                  "fixture has visible chest slot 3 populated",
                  "CHEST.C F0334:117-132");
}

static void test_c011_inventory_close_does_not_close_open_chest(void)
{
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat state;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(&state);
    accepted =
        DM1_V1_MirrorCandidateChestClosePendingPanel_AttemptInventoryClosePc34Compat(
            &state, &result);

    CHECK_REDMCSB(accepted == 0 && result.accepted == 0,
                  "C011 close is not accepted while G0299 is set",
                  "COMMAND.C F0380:2180-2183");
    CHECK_REDMCSB(result.command ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C011_CLOSE_INVENTORY_PC34_COMPAT,
                  "result records the C011 inventory-close command",
                  "COMMAND.C F0380:2180-2183");
    CHECK_REDMCSB(result.blockedByCandidate == 1 && result.ignored == 1,
                  "C011 is blocked by the pending candidate",
                  "COMMAND.C F0380:2180-2183");
    CHECK_REDMCSB(result.dispatchedF0355 == 0,
                  "blocked C011 does not dispatch F0355",
                  "PANEL.C F0355:2314-2318");
    CHECK_REDMCSB(result.dispatchedF0334 == 0,
                  "blocked C011 does not dispatch F0334",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.f0355ToggleCountAfter ==
                      result.f0355ToggleCountBefore,
                  "F0355 count is unchanged",
                  "PANEL.C F0355:2314-2318");
    CHECK_REDMCSB(result.f0334CloseCountAfter ==
                      result.f0334CloseCountBefore,
                  "F0334 close count is unchanged",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.blockedInventoryCloseCountAfter ==
                      result.blockedInventoryCloseCountBefore + 1,
                  "blocked close count increments once",
                  "COMMAND.C F0380:2180-2183");
    CHECK_REDMCSB(result.chestOpenPreserved == 1 &&
                      result.openChestAfter ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_OPEN_CHEST_PC34_COMPAT,
                  "G0426 open chest is preserved",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.chestSlotsPreserved == 1,
                  "G0425 visible chest slots are preserved",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.candidatePreserved == 1,
                  "candidate ordinal and party count are preserved",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.panelPreserved == 1,
                  "C040 panel state is preserved",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.inventoryPreserved == 1,
                  "inventory owner ordinal is preserved",
                  "PANEL.C F0355:2314-2318");
    CHECK_REDMCSB(result.noChestCloseSideEffects == 1,
                  "blocked C011 has no close/relink side effects",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(state.openChestThing == result.openChestBefore,
                  "state keeps the same open chest thing",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(state.chestSlots[0] == result.slot0Before &&
                      state.chestSlots[2] == result.slot2Before &&
                      state.chestSlots[3] == result.slot3Before,
                  "state keeps non-empty visible chest slots",
                  "CHEST.C F0334:117-132");
}

static void test_f0302_chest_slot_swap_while_c040_pending(void)
{
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat state;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateChestClosePendingPanel_SwapChestSlotPc34Compat(
        &state, 0, &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "C038 chest slot is accepted through F0302",
                  "COMMAND.C F0380:2174-2178");
    CHECK_REDMCSB(result.command ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C038_CHEST_SLOT_1_PC34_COMPAT,
                  "result records C038 chest-slot command",
                  "COMMAND.C F0380:2174-2178");
    CHECK_REDMCSB(result.requestedChestSlotIndex == 0,
                  "result records chest slot zero",
                  "CHAMPION.C F0302:688-690");
    CHECK_REDMCSB(result.dispatchedF0302 == 1,
                  "slot-box command dispatches F0302",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.f0302SlotDispatchCountAfter ==
                      result.f0302SlotDispatchCountBefore + 1,
                  "F0302 dispatch count increments once",
                  "COMMAND.C F0380:2174-2178");
    CHECK_REDMCSB(result.leaderHandSwapped == 1 &&
                      result.f0302SwapCountAfter ==
                          result.f0302SwapCountBefore + 1,
                  "F0302 swap count increments once",
                  "CHAMPION.C F0302:700-710");
    CHECK_REDMCSB(result.leaderHandBefore ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_LEADER_HAND_PC34_COMPAT,
                  "leader hand starts with fixture item",
                  "CHAMPION.C F0302:688");
    CHECK_REDMCSB(result.leaderHandAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT0_THING_PC34_COMPAT,
                  "leader hand receives the chest-slot item",
                  "CHAMPION.C F0302:704-706");
    CHECK_REDMCSB(result.slot0After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_LEADER_HAND_PC34_COMPAT,
                  "slot zero receives previous leader-hand item",
                  "CHAMPION.C F0302:708-710");
    CHECK_REDMCSB(result.slot1After == result.slot1Before &&
                      result.slot2After == result.slot2Before &&
                      result.slot3After == result.slot3Before,
                  "non-target visible chest slots are preserved",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.openChestAfter == result.openChestBefore,
                  "F0302 swap does not close G0426",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.dispatchedF0334 == 0 &&
                      result.f0334CloseCountAfter ==
                          result.f0334CloseCountBefore,
                  "F0302 swap does not dispatch F0334",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.noChestCloseSideEffects == 1,
                  "F0302 swap has no F0334 close/relink side effects",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.candidatePreserved == 1,
                  "candidate remains pending after chest-slot swap",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.panelPreserved == 1 && result.c040OpenAfter == 1,
                  "C040 remains open after chest-slot swap",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.inventoryPreserved == 1,
                  "inventory owner remains selected after chest-slot swap",
                  "PANEL.C F0355:2314-2318");
}

static void test_c162_cancel_closes_and_relinks_chest_once(void)
{
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat state;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateChestClosePendingPanel_CancelC040Pc34Compat(
        &state, &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "C162 cancel is accepted by the C040 panel",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.command ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C162_CANCEL_PC34_COMPAT,
                  "result records the C162 cancel command",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.explicitC040Cancel == 1,
                  "result records explicit C040 cancel",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.explicitC040CancelCountAfter ==
                      result.explicitC040CancelCountBefore + 1,
                  "C040 cancel count increments once",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.dispatchedF0355 == 1 &&
                      result.f0355ToggleCountAfter ==
                          result.f0355ToggleCountBefore + 1,
                  "C162 dispatches F0355 once",
                  "REVIVE.C F0282:746; PANEL.C F0355:2314-2318");
    CHECK_REDMCSB(result.dispatchedF0334 == 1 &&
                      result.f0334CloseCountAfter ==
                          result.f0334CloseCountBefore + 1,
                  "F0355 dispatches F0334 once",
                  "PANEL.C F0355:2314-2318; CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.openChestAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT,
                  "F0334 clears G0426 open chest",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.containerHeadAfter == result.slot0Before,
                  "F0334 writes the first non-empty slot to container head",
                  "CHEST.C F0334:123-127");
    CHECK_REDMCSB(result.chestFirstSlotWriteCountAfter ==
                      result.chestFirstSlotWriteCountBefore + 1,
                  "F0334 first-slot write count increments once",
                  "CHEST.C F0334:123-127");
    CHECK_REDMCSB(result.chestRelinkCountAfter ==
                      result.chestRelinkCountBefore + 2,
                  "F0334 relinks the remaining two non-empty slots",
                  "CHEST.C F0334:128-130");
    CHECK_REDMCSB(result.chestSlotClearCountAfter ==
                      result.chestSlotClearCountBefore + 3,
                  "F0334 clears the three non-empty visible slots",
                  "CHEST.C F0334:119-122");
    CHECK_REDMCSB(result.slot0After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT &&
                      result.slot2After ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT &&
                      result.slot3After ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT,
                  "all non-empty visible slots are cleared",
                  "CHEST.C F0334:119-122");
    CHECK_REDMCSB(result.closeRepackedNonEmptySlots == 1,
                  "result summarizes F0334 visible-slot repack",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.candidateOrdinalAfter == 0u &&
                      result.candidateClearCountAfter ==
                          result.candidateClearCountBefore + 1,
                  "C162 clears G0299 after F0355",
                  "REVIVE.C F0282:746-747");
    CHECK_REDMCSB(result.partyCountAfter == result.partyCountBefore - 1 &&
                      result.partyDecrementCountAfter ==
                          result.partyDecrementCountBefore + 1,
                  "C162 decrements party count once",
                  "REVIVE.C F0282:751-757");
    CHECK_REDMCSB(result.inventoryOrdinalAfter == 0u,
                  "F0355 clears inventory owner ordinal",
                  "PANEL.C F0355:2314-2317");
    CHECK_REDMCSB(result.c040OpenAfter == 0 && result.panelContentAfter == 0,
                  "C040 panel is closed after explicit cancel",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.leaderHandAfter == result.leaderHandBefore,
                  "C162 close does not swap leader hand",
                  "CHAMPION.C F0302:688-710");
}

int main(void)
{
    test_source_lock_metadata();
    test_fixture_starts_with_c040_and_open_chest();
    test_c011_inventory_close_does_not_close_open_chest();
    test_f0302_chest_slot_swap_while_c040_pending();
    test_c162_cancel_closes_and_relinks_chest_once();

    printf("assertions=%d\n", gTests);
    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_chest_close_pending_panel_pc34_compat "
               "passed=%d/%d\n",
               gPasses,
               gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_chest_close_pending_panel_pc34_compat "
           "assertions=%d\n",
           gTests);
    return 0;
}
