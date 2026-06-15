#include "dm1_v1_mirror_candidate_reincarnate_rearm_pc34_compat.h"

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

static Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat
base_candidate_state(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state;

    DM1_V1_MirrorCandidateReincarnateRearm_InitPc34Compat(&state);
    return state;
}

static void test_c161_reincarnate_clears_g0299_and_rearms_mirror(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state =
        base_candidate_state();
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat result;
    int changed;

    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u &&
                      state.partyChampionCount == 2,
                  "C127/F0280 fixture publishes the appended candidate",
                  "MOVESENS.C:1501-1503; REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.c040PanelOpen == 1 &&
                      state.c040PanelPixelsDrawn == 1,
                  "C040 resurrect/reincarnate panel starts drawn",
                  "PANEL.C F0346:1619-1635; PANEL.C F0347");

    changed = DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 1 && result.validPanelCommand == 1,
                  "C161 reincarnate is a live C040 panel command, not a no-op",
                  "COMMAND.C:508-511; DEFS.H:4041-4042 C160/C161/C162");
    CHECK_REDMCSB(result.reincarnated == 1 && result.resurrected == 0,
                  "C161 follows the reincarnate branch, not C160 resurrect",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.candidateChampionOrdinalBefore == 2u &&
                      state.candidateChampionOrdinal == 0u &&
                      result.candidateChampionOrdinalAfter == 0u,
                  "C161 clears G0299 after the live command",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.candidateCleared == 1 &&
                      result.c040PanelCleared == 1 &&
                      state.c040PanelOpen == 0,
                  "C040 panel is closed after C161 finalizes",
                  "REVIVE.C F0282:785-806; PANEL.C F0347");
    CHECK_REDMCSB(result.mirrorSensorDisabled == 1 &&
                      state.firstMirrorSensorDisabled == 1,
                  "C161 disables the first mirror-square sensor",
                  "REVIVE.C F0282:785-806; DUNVIEW.C:8488-8533");
    CHECK_REDMCSB(result.currentHealthBefore == 80 &&
                      result.currentHealthAfter == 40 &&
                      state.champions[1].currentHealth == 40,
                  "C161 halves current health for reincarnation",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.maximumHealthBefore == 160 &&
                      result.maximumHealthAfter == 80,
                  "C161 halves maximum health",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.currentStaminaBefore == 60 &&
                      result.currentStaminaAfter == 30 &&
                      result.maximumStaminaAfter == 60,
                  "C161 halves stamina",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.currentManaBefore == 30 &&
                      result.currentManaAfter == 15 &&
                      result.maximumManaAfter == 30,
                  "C161 halves mana",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.skillBytesNonZeroBefore == 1 &&
                      result.skillBytesNonZeroAfter == 0 &&
                      result.skillsCleared == 1,
                  "C161 clears candidate skills before rearming",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.newFrontD1cMirrorChampionOrdinal == 2 &&
                      state.frontD1cMirrorChampionOrdinal == 2,
                  "front D1C mirror ordinal is rearmed to the reincarnated candidate",
                  "DUNVIEW.C:3913-3928; DUNVIEW.C:8488-8533");
    CHECK_REDMCSB(result.frontD1cPortraitIndex == 11 &&
                      result.mirrorRouteRearmed == 1,
                  "D1L/D1R-before-D1C draw order still reaches the candidate portrait",
                  "DUNVIEW.C:3913-3928; C127 champion portrait sensor");
}

static void test_c161_does_not_enter_leader_hand_routes(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state =
        base_candidate_state();
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat result;
    int leaderHandBefore = state.leaderHandThingOrdinal;
    int leaderIndexBefore = state.leaderIndex;

    (void)DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(result.leaderHandPutRouteEntered == 0,
                  "C161 does not enter F0297 put-object-in-leader-hand",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(result.leaderHandRemoveRouteEntered == 0,
                  "C161 does not enter F0298 remove-object-from-leader-hand",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(result.slotBoxRouteEntered == 0,
                  "C161 does not enter F0302 slot-box dispatch",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(state.leaderHandThingOrdinal == leaderHandBefore &&
                      result.previousLeaderHandThingOrdinal == leaderHandBefore &&
                      result.newLeaderHandThingOrdinal == leaderHandBefore,
                  "leader-hand occupant is unchanged by C161",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(state.leaderIndex == leaderIndexBefore &&
                      result.newLeaderIndex == leaderIndexBefore,
                  "C161 does not route through leader-hand leader changes",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
}

static void test_c160_resurrect_contract_remains_distinct(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state =
        base_candidate_state();
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat result;
    int changed;

    state.champions[1].currentHealth = 0;
    state.champions[1].maximumHealth = 160;
    state.champions[1].skillBytesNonZero = 1;
    changed = DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_RESURRECT_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 1 && result.validPanelCommand == 1,
                  "C160 remains a valid C040 panel command",
                  "COMMAND.C:508-511; DEFS.H:4041-4042 C160/C161/C162");
    CHECK_REDMCSB(result.resurrected == 1 &&
                      result.reincarnated == 0 &&
                      result.resurrectContractPreserved == 1,
                  "C160 stays resurrect and is not consumed by the C161 gate",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 0u &&
                      result.candidateCleared == 1,
                  "C160 still clears G0299 on finalization",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(state.champions[1].currentHealth == 1 &&
                      result.currentHealthAfter == 1,
                  "C160 resurrect contract restores one hit point",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(state.champions[1].maximumHealth == 160 &&
                      result.maximumHealthAfter == 160,
                  "C160 does not apply C161 health halving",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(state.champions[1].skillBytesNonZero == 1 &&
                      result.skillBytesNonZeroAfter == 1,
                  "C160 does not clear skills like reincarnate",
                  "REVIVE.C F0282:806-835");
    CHECK_REDMCSB(result.mirrorRouteRearmed == 1 &&
                      result.frontD1cPortraitIndex == 11,
                  "C160 still rearms the mirror route",
                  "DUNVIEW.C:3913-3928; DUNVIEW.C:8488-8533");
}

static void test_candidate_panel_gate_reopens_after_c161(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state =
        base_candidate_state();
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat beforeStatus;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat beforeAction;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat afterStatus;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat afterInventory;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat afterSpell;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat afterAction;
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat reincarnate;

    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_STATUS_BOX_0_PC34_COMPAT,
        &beforeStatus);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &beforeAction);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT,
        &reincarnate);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_STATUS_BOX_0_PC34_COMPAT,
        &afterStatus);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_INVENTORY_TOGGLE_0_PC34_COMPAT,
        &afterInventory);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_SPELL_AREA_COMMAND_PC34_COMPAT,
        &afterSpell);
    (void)DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &afterAction);

    CHECK_REDMCSB(beforeStatus.blockedByCandidatePanel == 1 &&
                      beforeStatus.statusBoxAllowed == 0,
                  "G0299 blocks status-box dispatch while C040 is open",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(beforeAction.blockedByCandidatePanel == 1 &&
                      beforeAction.actionAreaAllowed == 0,
                  "G0299 blocks action-area dispatch while C040 is open",
                  "COMMAND.C:2302-2311");
    CHECK_REDMCSB(afterStatus.panelC040Closed == 1 &&
                      afterStatus.statusBoxAllowed == 1,
                  "status-box dispatch is re-enabled after C161 clears G0299",
                  "COMMAND.C:2159-2181; REVIVE.C F0282:785-806");
    CHECK_REDMCSB(afterInventory.inventoryToggleAllowed == 1,
                  "inventory-toggle dispatch is re-enabled after C161",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(afterSpell.spellAreaAllowed == 1,
                  "spell-area dispatch is re-enabled after C161",
                  "COMMAND.C:2302-2311");
    CHECK_REDMCSB(afterAction.actionAreaAllowed == 1,
                  "action-area dispatch is re-enabled after C161",
                  "COMMAND.C:2302-2311");
}

static void test_c161_requires_candidate_state(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state =
        base_candidate_state();
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat result;
    int changed;

    state.candidateChampionOrdinal = 0u;
    changed = DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.ignoredNoCandidate == 1,
                  "C161 is ignored when G0299 has no candidate",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.reincarnated == 0 &&
                      result.candidateChampionOrdinalAfter == 0u,
                  "candidate-less C161 is not a reincarnate no-op success",
                  "REVIVE.C F0282:785-806; COMMAND.C:508-511");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *evidence =
        DM1_V1_MirrorCandidateReincarnateRearm_SourceEvidencePc34Compat();

    CHECK_REDMCSB(evidence != NULL,
                  "source evidence string is available",
                  "MOVESENS.C:1501-1503; REVIVE.C F0282:785-806");
    CHECK_REDMCSB(strstr(evidence, "MOVESENS.C:1501-1503") != NULL,
                  "evidence cites champion portrait sensor route",
                  "MOVESENS.C:1501-1503");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0280:272-276") != NULL,
                  "evidence cites candidate publish through G0299",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282:744-758") != NULL,
                  "evidence cites cancel half of F0282",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282:785-806") != NULL,
                  "evidence cites live C160/C161 clear path",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C:508-511") != NULL,
                  "evidence cites G0457 panel button mapping",
                  "COMMAND.C:508-511");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C:2159-2181") != NULL,
                  "evidence cites status/inventory !G0299 gate",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C:2302-2311") != NULL,
                  "evidence cites spell/action !G0299 gate",
                  "COMMAND.C:2302-2311");
    CHECK_REDMCSB(strstr(evidence, "PANEL.C F0346:1619-1635") != NULL &&
                      strstr(evidence, "PANEL.C F0347") != NULL,
                  "evidence cites C040 blit to C101 panel",
                  "PANEL.C F0346:1619-1635; PANEL.C F0347");
    CHECK_REDMCSB(strstr(evidence, "DUNVIEW.C:3913-3928") != NULL &&
                      strstr(evidence, "DUNVIEW.C:8488-8533") != NULL,
                  "evidence cites D1L/D1R before D1C draw order",
                  "DUNVIEW.C:3913-3928,8488-8533");
    CHECK_REDMCSB(strstr(evidence, "CHAMPION.C F0297/F0298/F0302") != NULL,
                  "evidence cites leader-hand put/remove/slot routes",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(strstr(evidence, "DEFS.H:4041-4042") != NULL &&
                      strstr(evidence, "C160/C161/C162") != NULL,
                  "evidence cites command constants",
                  "DEFS.H:4041-4042 C160/C161/C162");
}

int main(void)
{
    test_c161_reincarnate_clears_g0299_and_rearms_mirror();
    test_c161_does_not_enter_leader_hand_routes();
    test_c160_resurrect_contract_remains_distinct();
    test_candidate_panel_gate_reopens_after_c161();
    test_c161_requires_candidate_state();
    test_source_evidence_mentions_all_anchors();

    printf("PASS dm1_v1_mirror_candidate_reincarnate_rearm_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
