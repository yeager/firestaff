#include "dm1_v1_mirror_candidate_open_then_reselect_pc34_compat.h"

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
    const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateOpenThenReselect_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence accessor returns metadata",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1,
                  "fixture is marked contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->contractScope, "contract_only=1") != NULL,
                  "contract scope contains the explicit marker",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->contractScope, "no real assets") != NULL,
                  "contract scope rejects real-asset parity claims",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->contractScope, "pixel parity") != NULL,
                  "contract scope rejects pixel parity claims",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->commandNameRowAnchor, "COMMAND.C:484-488") != NULL,
                  "C159 name-row table is cited",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(strstr(e->commandNameRowAnchor, "C159") != NULL &&
                      strstr(e->commandNameRowAnchor, "C016") != NULL,
                  "C159 to C016 mapping is cited",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(strstr(e->commandNameRowAnchor, "C160") != NULL &&
                      strstr(e->commandNameRowAnchor, "C017") != NULL,
                  "C160 to C017 mapping is cited",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(strstr(e->commandF0359DispatchAnchor,
                         "F0359:1985-1990") != NULL,
                  "F0359 C040 dispatch is cited",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandF0359DispatchAnchor, "leader-empty") != NULL,
                  "F0359 leader-hand guard is cited",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelMouseAnchor, "508-511") != NULL,
                  "C040 panel mouse table is cited",
                  e->commandPanelMouseAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelMouseAnchor, "C160") != NULL &&
                      strstr(e->commandPanelMouseAnchor, "C162") != NULL,
                  "panel commands C160..C162 are cited",
                  e->commandPanelMouseAnchor);
    CHECK_REDMCSB(strstr(e->panelF0354ChampionSwitchAnchor,
                         "F0354:2208-2240") != NULL,
                  "F0354 selected portrait redraw is cited",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(strstr(e->panelF0354ChampionSwitchAnchor,
                         "champion-switch") != NULL,
                  "F0354 anchor explains champion switch cadence",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(strstr(e->chestF0333SameOpenAnchor, "F0333:30-32") != NULL,
                  "F0333 same-open no-op is cited",
                  e->chestF0333SameOpenAnchor);
    CHECK_REDMCSB(strstr(e->chestF0333SameOpenAnchor, "same-open") != NULL,
                  "F0333 same-open contract is named",
                  e->chestF0333SameOpenAnchor);
    CHECK_REDMCSB(strstr(e->chestF0334CloseRewriteAnchor,
                         "F0334:113-132") != NULL,
                  "F0334 close rewrite is cited",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(strstr(e->chestF0334CloseRewriteAnchor,
                         "first-slot") != NULL,
                  "F0334 first-slot rewrite is cited",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(strstr(e->reviveF0280NoPendingAnchor,
                         "F0280:124-132") != NULL,
                  "F0280 no-pending publication guard is cited",
                  e->reviveF0280NoPendingAnchor);
    CHECK_REDMCSB(strstr(e->reviveF0282ClearAnchor,
                         "F0282:744-758") != NULL,
                  "F0282 cancel clear path is cited",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(strstr(e->commandGuardAnchor,
                         "2159-2181") != NULL,
                  "status/inventory guard range is cited",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandGuardAnchor,
                         "2302-2311") != NULL,
                  "spell/action guard range is cited",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandGuardAnchor,
                         "2366-2370") != NULL,
                  "save guard range is cited",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandGuardAnchor, "!G0299") != NULL,
                  "!G0299 guard marker is cited",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "open then different champion") != NULL,
                  "non-overlap note identifies open-then-reselect",
                  e->nonOverlapNote);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "select/click/cancel") != NULL,
                  "non-overlap note distinguishes adjacent slices",
                  e->nonOverlapNote);
}

static void test_init_contract_state(void)
{
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat state;

    DM1_V1_MirrorCandidateOpenThenReselect_InitPc34Compat(&state);

    CHECK_REDMCSB(state.contractOnly == 1,
                  "init sets contract-only state",
                  "contract");
    CHECK_REDMCSB(state.partyChampionCount == 2,
                  "fixture starts with two champions",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.selectedChampionIndex == 0,
                  "champion A starts selected",
                  "PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 0u,
                  "fixture starts with no pending candidate",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.c040PanelOpen == 0,
                  "fixture starts with C040 closed",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.panelOwnerChampionIndex ==
                      DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_NONE_PC34_COMPAT,
                  "fixture starts with no panel owner",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.leaderHandThing == 0x7a11u,
                  "fixture records the carried hand object",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.leaderHandEmpty == 1,
                  "fixture satisfies the leader-empty C040 guard",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.champions[0].present == 1 &&
                      state.champions[1].present == 1,
                  "champion A and B are present",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(state.champions[0].championOrdinal == 1u &&
                      state.champions[1].championOrdinal == 2u,
                  "champion ordinals are stable",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(state.champions[0].slotFingerprint !=
                      state.champions[1].slotFingerprint,
                  "A and B have distinct slot state",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(state.f0354SwitchCount == 0 &&
                      state.f0354RedrawCount == 0,
                  "redraw cadence starts at zero",
                  "PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(state.f0333SameOpenNoopCount == 0 &&
                      state.f0334CloseRewriteCount == 0,
                  "chest/panel close counters start at zero",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(state.assetLoadCount == 0 &&
                      state.pixelParityClaimCount == 0,
                  "fixture does not load assets or claim pixel parity",
                  "contract_only=1");
}

static void test_manual_deadzone_and_no_pending_paths(void)
{
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat state;
    int changed;
    int rejected;

    DM1_V1_MirrorCandidateOpenThenReselect_InitPc34Compat(&state);
    rejected =
        DM1_V1_MirrorCandidateOpenThenReselect_ClickResurrectPc34Compat(
            &state);
    CHECK_REDMCSB(rejected == 0,
                  "C160 resurrect is rejected without pending G0299",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.noPendingResurrectRejectCount == 1,
                  "no-pending resurrect rejection is counted",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 0u,
                  "no-pending resurrect leaves G0299 clear",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.c040PanelOpen == 0,
                  "no-pending resurrect does not open C040",
                  "COMMAND.C F0359:1985-1990");

    changed = DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
        &state, 0);
    CHECK_REDMCSB(changed == 0,
                  "selecting the already selected champion is a deadzone",
                  "PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(state.sameChampionNoopCount == 1,
                  "same-champion deadzone is counted",
                  "PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(state.f0354SwitchCount == 0 &&
                      state.f0354RedrawCount == 0,
                  "deadzone does not redraw",
                  "PANEL.C F0354:2208-2240");
    CHECK_REDMCSB(state.leaderHandThing == 0x7a11u,
                  "deadzone preserves hand-carry",
                  "COMMAND.C F0359:1985-1990");
}

static void test_open_then_reselect_contract(void)
{
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat state;
    Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat result;
    const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateOpenThenReselect_EvidencePc34Compat();
    int ok;

    DM1_V1_MirrorCandidateOpenThenReselect_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateOpenThenReselect_RunPc34Compat(
        &state, &result);

    CHECK_REDMCSB(ok == 1 && result.ok == 1,
                  "full open-then-reselect contract passes",
                  e->contractScope);
    CHECK_REDMCSB(result.evidence == e,
                  "result carries source-lock evidence",
                  "metadata");
    CHECK_REDMCSB(result.championAIndex == 0 &&
                      result.championBIndex == 1,
                  "flow uses champion A then champion B",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.c159MappedToC016A == 1,
                  "champion A C159 row maps to C016",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.c159MappedToC017B == 1,
                  "champion B name row maps to C017",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(result.aPanelOpened == 1,
                  "candidate panel opens on champion A",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.g0299AfterAOpen == 1u,
                  "G0299 tracks champion A after A open",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.sameOpenNoopPreserved == 1,
                  "opening A again is the F0333 same-open no-op",
                  e->chestF0333SameOpenAnchor);
    CHECK_REDMCSB(state.f0333SameOpenNoopCount == 1,
                  "same-open no-op counter increments once",
                  e->chestF0333SameOpenAnchor);
    CHECK_REDMCSB(result.sameChampionDeadzoneNoop == 1,
                  "selecting A while A is selected is a no-op",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(state.sameChampionNoopCount == 1,
                  "same-champion deadzone counter increments once",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.bSelectedViaF0354 == 1,
                  "champion B is selected through F0354 cadence",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.f0354SwitchCountAfterBSelect ==
                      result.f0354SwitchCountBefore + 1,
                  "B select increments switch cadence once",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.f0354RedrawCountAfterBSelect ==
                      result.f0354RedrawCountBefore + 1,
                  "B select increments redraw cadence once",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.closeRewriteRanBeforeBOpen == 1,
                  "A panel is closed/re-written before B opens",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.f0334CloseRewriteCountAfterBOpen ==
                      result.f0334CloseRewriteCountBefore + 1,
                  "F0334 close rewrite count advances for A to B",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.bPanelReopened == 1,
                  "candidate panel reopens on champion B",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.bPanelUsesBSlotState == 1,
                  "B reopen uses B slot state",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.bSlotAfterReopen == result.bSlotBefore,
                  "B slot fingerprint is preserved",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.g0299AfterBOpen == 2u,
                  "G0299 tracks champion B after B open",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.bCancelClearedPending == 1,
                  "cancel on B clears the pending candidate",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(result.g0299AfterBCancel == 0u,
                  "G0299 is clear after B cancel",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(result.f0282CancelCountAfterB ==
                      result.f0282CancelCountBefore + 1,
                  "B cancel advances the F0282 cancel count once",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(result.noPendingResurrectRejected == 1,
                  "C160 with no pending candidate is rejected",
                  e->reviveF0280NoPendingAnchor);
    CHECK_REDMCSB(state.noPendingResurrectRejectCount == 1,
                  "no-pending resurrect path was exercised once",
                  e->reviveF0280NoPendingAnchor);
    CHECK_REDMCSB(result.reopenedAUsesPreviousAState == 1,
                  "reopening A restores A's previous slot state",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.aSlotAfterReopen == result.aSlotBefore,
                  "A slot fingerprint survives B reselect",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.aSlotAfterReopen != result.bSlotBefore,
                  "A reopen does not use B slot fingerprint",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(result.g0299AfterAReopen == 1u,
                  "G0299 tracks champion A after A reopens",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.noBRedrawLeakedIntoA == 1,
                  "B redraw generation does not leak into A",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(state.champions[0].redrawGeneration == 1 &&
                      state.champions[1].redrawGeneration == 1,
                  "A and B redraw generations are independent",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(state.lastRedrawChampionIndex == 0,
                  "last redraw belongs to A after A reselect",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.redrawCadencePreserved == 1,
                  "redraw cadence is one redraw per real champion switch",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.f0354SwitchCountAfterAReselect ==
                      result.f0354SwitchCountBefore + 2,
                  "A reselect is the second real switch",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.f0354RedrawCountAfterAReselect ==
                      result.f0354RedrawCountBefore + 2,
                  "A reselect is the second redraw",
                  e->panelF0354ChampionSwitchAnchor);
    CHECK_REDMCSB(result.handCarryPreserved == 1,
                  "hand-carry is preserved across open/reselect/cancel",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.leaderHandBefore == result.leaderHandAfter,
                  "leader-hand thing is unchanged",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.noLeaderHandRoutes == 1,
                  "leader-hand put/remove routes are not entered",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(result.noSlotRoutes == 1,
                  "slot-box routes are not entered",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.guardsBlockedWhileG0299 == 1,
                  "!G0299 guarded inputs are blocked while panel is pending",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.blockedStatusBoxCountAfter == 3,
                  "status-box guard was probed on each successful open",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.blockedInventoryToggleCountAfter == 3,
                  "inventory-toggle guard was probed on each successful open",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.blockedSpellRuneCountAfter == 3,
                  "spell-rune guard was probed on each successful open",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.blockedActionAreaCountAfter == 3,
                  "action-area guard was probed on each successful open",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(result.blockedSaveCountAfter == 3,
                  "save guard was probed on each successful open",
                  e->commandGuardAnchor);
    CHECK_REDMCSB(state.c159RowClickCount == 4,
                  "C159/name-row flow includes A open, same-open, B open, A reopen",
                  e->commandNameRowAnchor);
    CHECK_REDMCSB(state.f0359PanelDispatchCount == 3,
                  "F0359 dispatches three successful panel opens",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.f0334CloseRewriteCount == 2,
                  "close rewrite runs for A-to-B and B cancel",
                  e->chestF0334CloseRewriteAnchor);
    CHECK_REDMCSB(state.f0282CancelCount == 1,
                  "only B cancel enters F0282 cancel path",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(state.champions[0].candidateOpenCount == 2,
                  "champion A opened before and after B",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.champions[1].candidateOpenCount == 1,
                  "champion B opened once",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.champions[1].cancelCount == 1,
                  "champion B owns the cancel",
                  e->reviveF0282ClearAnchor);
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "final A candidate panel is open",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.panelOwnerChampionIndex == 0,
                  "final panel owner is A",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_M568_PANEL_PC34_COMPAT,
                  "final panel content is M568",
                  e->commandF0359DispatchAnchor);
    CHECK_REDMCSB(state.c040Graphic ==
                      DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C040_GRAPHIC_PC34_COMPAT,
                  "final panel graphic is C040",
                  e->commandPanelMouseAnchor);
    CHECK_REDMCSB(result.contractOnly == 1,
                  "result keeps the contract-only marker",
                  e->contractScope);
    CHECK_REDMCSB(result.noAssetsOrPixelParity == 1,
                  "result confirms no assets or pixel parity",
                  e->contractScope);
}

int main(void)
{
    test_source_lock_metadata();
    test_init_contract_state();
    test_manual_deadzone_and_no_pending_paths();
    test_open_then_reselect_contract();

    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_open_then_reselect_pc34_compat "
               "%d/%d assertions\n",
               gPasses, gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_open_then_reselect_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return 0;
}
