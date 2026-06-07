#include "dm1_v1_mirror_candidate_reincarnate_rearm_pc34_compat.h"

#include <string.h>

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CHAMPION_COUNT_PC34_COMPAT;
}

static int front_d1c_portrait_index(
    const Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state)
{
    int championIndex;

    if (!state || state->frontD1cMirrorChampionOrdinal == 0) {
        return DM1_V1_MIRROR_CANDIDATE_REINCARNATE_NONE_PC34_COMPAT;
    }
    /* ReDMCSB: DUNVIEW.C:3913-3928 draws D1L/D1R before D1C and then
     * decrements the one-based C127 champion portrait ordinal for D1C. */
    championIndex = state->frontD1cMirrorChampionOrdinal - 1;
    if (!valid_champion_index(championIndex) ||
        !state->champions[championIndex].present) {
        return DM1_V1_MIRROR_CANDIDATE_REINCARNATE_NONE_PC34_COMPAT;
    }
    return state->champions[championIndex].portraitOrdinal;
}

void DM1_V1_MirrorCandidateReincarnateRearm_InitPc34Compat(
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    /* ReDMCSB: MOVESENS.C:1501-1503 routes a C127 champion portrait sensor
     * to REVIVE.C F0280, and REVIVE.C F0280:272-276 publishes G0299 as the
     * appended party ordinal while the C040 panel owns input. */
    state->partyChampionCount = 2;
    state->candidateChampionOrdinal = 2u;
    state->inventoryChampionOrdinal = 2u;
    state->leaderIndex = 0;
    state->frontD1cMirrorChampionOrdinal = 2;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn = 1;
    state->leaderHandThingOrdinal = 77;

    state->champions[0].present = 1;
    state->champions[0].currentHealth = 42;
    state->champions[0].maximumHealth = 84;
    state->champions[0].currentStamina = 50;
    state->champions[0].maximumStamina = 100;
    state->champions[0].currentMana = 12;
    state->champions[0].maximumMana = 24;
    state->champions[0].portraitOrdinal = 7;

    state->champions[1].present = 1;
    state->champions[1].currentHealth = 80;
    state->champions[1].maximumHealth = 160;
    state->champions[1].currentStamina = 60;
    state->champions[1].maximumStamina = 120;
    state->champions[1].currentMana = 30;
    state->champions[1].maximumMana = 60;
    state->champions[1].portraitOrdinal = 11;
    state->champions[1].skillBytesNonZero = 1;
}

static void result_init(
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat *result,
    const Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int command)
{
    int candidateIndex;

    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->command = command;
    result->candidateChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_NONE_PC34_COMPAT;
    result->previousLeaderIndex = state ? state->leaderIndex :
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_NONE_PC34_COMPAT;
    result->newLeaderIndex = result->previousLeaderIndex;
    result->previousLeaderHandThingOrdinal =
        state ? state->leaderHandThingOrdinal : 0;
    result->newLeaderHandThingOrdinal = result->previousLeaderHandThingOrdinal;
    result->previousFrontD1cMirrorChampionOrdinal =
        state ? state->frontD1cMirrorChampionOrdinal : 0;
    result->newFrontD1cMirrorChampionOrdinal =
        result->previousFrontD1cMirrorChampionOrdinal;
    result->frontD1cPortraitIndex = front_d1c_portrait_index(state);
    if (!state) {
        return;
    }
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    if (state->candidateChampionOrdinal == 0u) {
        return;
    }
    candidateIndex = (int)state->candidateChampionOrdinal - 1;
    result->candidateChampionIndex = candidateIndex;
    if (valid_champion_index(candidateIndex)) {
        const Dm1V1MirrorCandidateReincarnateChampionPc34Compat *champion =
            &state->champions[candidateIndex];
        result->currentHealthBefore = champion->currentHealth;
        result->currentHealthAfter = champion->currentHealth;
        result->maximumHealthBefore = champion->maximumHealth;
        result->maximumHealthAfter = champion->maximumHealth;
        result->currentStaminaBefore = champion->currentStamina;
        result->currentStaminaAfter = champion->currentStamina;
        result->maximumStaminaBefore = champion->maximumStamina;
        result->maximumStaminaAfter = champion->maximumStamina;
        result->currentManaBefore = champion->currentMana;
        result->currentManaAfter = champion->currentMana;
        result->maximumManaBefore = champion->maximumMana;
        result->maximumManaAfter = champion->maximumMana;
        result->skillBytesNonZeroBefore = champion->skillBytesNonZero;
        result->skillBytesNonZeroAfter = champion->skillBytesNonZero;
    }
}

static void publish_finalize_result(
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int candidateIndex,
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat *result)
{
    state->candidateChampionOrdinal = 0u;
    state->c040PanelOpen = 0;
    state->c040PanelPixelsDrawn = 0;
    state->firstMirrorSensorDisabled = 1;
    state->frontD1cMirrorChampionOrdinal = candidateIndex + 1;

    result->candidateCleared = 1;
    result->c040PanelCleared = 1;
    result->mirrorSensorDisabled = 1;
    result->mirrorRouteRearmed = 1;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->newLeaderIndex = state->leaderIndex;
    result->newLeaderHandThingOrdinal = state->leaderHandThingOrdinal;
    result->leaderHandPutRouteEntered = state->leaderHandPutRouteCount != 0;
    result->leaderHandRemoveRouteEntered = state->leaderHandRemoveRouteCount != 0;
    result->slotBoxRouteEntered = state->slotBoxRouteCount != 0;
    result->newFrontD1cMirrorChampionOrdinal =
        state->frontD1cMirrorChampionOrdinal;
    result->frontD1cPortraitIndex = front_d1c_portrait_index(state);
}

int DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat *outResult)
{
    int candidateIndex;
    Dm1V1MirrorCandidateReincarnateChampionPc34Compat *champion;

    result_init(outResult, state, command);
    if (!state || !outResult) {
        return 0;
    }
    if (command !=
            DM1_V1_MIRROR_CANDIDATE_REINCARNATE_RESURRECT_COMMAND_PC34_COMPAT &&
        command != DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT) {
        outResult->ignoredWrongCommand = 1;
        return 0;
    }
    if (state->candidateChampionOrdinal == 0u) {
        /* ReDMCSB: COMMAND.C:508-511 maps G0457 C040 panel buttons, but
         * REVIVE.C F0282 only has a live candidate while G0299 is non-zero. */
        outResult->ignoredNoCandidate = 1;
        return 0;
    }

    candidateIndex = (int)state->candidateChampionOrdinal - 1;
    if (!valid_champion_index(candidateIndex) ||
        candidateIndex >= state->partyChampionCount ||
        !state->champions[candidateIndex].present) {
        outResult->ignoredNoCandidate = 1;
        return 0;
    }
    outResult->validPanelCommand = 1;
    champion = &state->champions[candidateIndex];

    if (command ==
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_RESURRECT_COMMAND_PC34_COMPAT) {
        /* ReDMCSB: DEFS.H:4041-4042 names C160/C161/C162 (local source
         * line set: 338-340); C160 stays the resurrect branch and does not
         * execute the REVIVE.C F0282:806-835 reincarnate stat/skill path. */
        champion->currentHealth = 1;
        publish_finalize_result(state, candidateIndex, outResult);
        outResult->resurrected = 1;
        outResult->resurrectContractPreserved = 1;
        outResult->currentHealthAfter = champion->currentHealth;
        outResult->maximumHealthAfter = champion->maximumHealth;
        outResult->skillBytesNonZeroAfter = champion->skillBytesNonZero;
        return 1;
    }

    /* ReDMCSB: REVIVE.C F0282:785-806 clears G0299, disables the first
     * mirror-square sensor, then the C161 branch at 806-835 renames,
     * clears skills, and halves the DM1 V1 health/stamina/mana fields. */
    champion->currentHealth >>= 1;
    champion->maximumHealth >>= 1;
    champion->currentStamina >>= 1;
    champion->maximumStamina >>= 1;
    champion->currentMana >>= 1;
    champion->maximumMana >>= 1;
    champion->skillBytesNonZero = 0;
    publish_finalize_result(state, candidateIndex, outResult);
    outResult->reincarnated = 1;
    outResult->currentHealthAfter = champion->currentHealth;
    outResult->maximumHealthAfter = champion->maximumHealth;
    outResult->currentStaminaAfter = champion->currentStamina;
    outResult->maximumStaminaAfter = champion->maximumStamina;
    outResult->currentManaAfter = champion->currentMana;
    outResult->maximumManaAfter = champion->maximumMana;
    outResult->skillBytesNonZeroAfter = champion->skillBytesNonZero;
    outResult->skillsCleared =
        outResult->skillBytesNonZeroBefore != 0 &&
        outResult->skillBytesNonZeroAfter == 0;
    /* ReDMCSB: CHAMPION.C F0297/F0298/F0302:243-285,662-706 are leader-hand
     * put/remove/slot routes; C161 is a C040 panel command and must not enter
     * those paths. */
    outResult->leaderHandPutRouteEntered = state->leaderHandPutRouteCount != 0;
    outResult->leaderHandRemoveRouteEntered = state->leaderHandRemoveRouteCount != 0;
    outResult->slotBoxRouteEntered = state->slotBoxRouteCount != 0;
    return 1;
}

int DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
    const Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat *outResult)
{
    if (outResult) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->command = command;
    }
    if (!state || !outResult) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C:2159-2181 gates status boxes and inventory toggles
     * on !G0299, and COMMAND.C:2302-2311 applies the same gate to spell and
     * action dispatch while the C040 candidate panel is live. */
    outResult->panelC040Closed = state->candidateChampionOrdinal == 0u;
    outResult->blockedByCandidatePanel = !outResult->panelC040Closed;
    outResult->statusBoxAllowed = outResult->panelC040Closed &&
        command >= DM1_V1_MIRROR_CANDIDATE_REINCARNATE_STATUS_BOX_0_PC34_COMPAT &&
        command < DM1_V1_MIRROR_CANDIDATE_REINCARNATE_STATUS_BOX_0_PC34_COMPAT +
            DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CHAMPION_COUNT_PC34_COMPAT;
    outResult->inventoryToggleAllowed = outResult->panelC040Closed &&
        command >=
            DM1_V1_MIRROR_CANDIDATE_REINCARNATE_INVENTORY_TOGGLE_0_PC34_COMPAT &&
        command <=
            DM1_V1_MIRROR_CANDIDATE_REINCARNATE_INVENTORY_TOGGLE_0_PC34_COMPAT +
            DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CHAMPION_COUNT_PC34_COMPAT;
    outResult->spellAreaAllowed = outResult->panelC040Closed &&
        command == DM1_V1_MIRROR_CANDIDATE_REINCARNATE_SPELL_AREA_COMMAND_PC34_COMPAT;
    outResult->actionAreaAllowed = outResult->panelC040Closed &&
        command == DM1_V1_MIRROR_CANDIDATE_REINCARNATE_ACTION_AREA_COMMAND_PC34_COMPAT;
    return outResult->statusBoxAllowed || outResult->inventoryToggleAllowed ||
        outResult->spellAreaAllowed || outResult->actionAreaAllowed;
}

const char *DM1_V1_MirrorCandidateReincarnateRearm_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB MOVESENS.C:1501-1503 C127 champion portrait sensor reaches "
           "REVIVE.C F0280; REVIVE.C F0280:272-276 publishes the candidate "
           "through G0299; REVIVE.C F0282:744-758 handles cancel and "
           "REVIVE.C F0282:785-806 clears G0299 before the C161 reincarnate "
           "branch; COMMAND.C:508-511 maps C040 panel buttons through G0457; "
           "COMMAND.C:2159-2181 gates status boxes and inventory toggles on "
           "!G0299; COMMAND.C:2302-2311 gates spell/action dispatch on !G0299; "
           "PANEL.C F0346:1619-1635 and PANEL.C F0347 draw C040 to C101; "
           "DUNVIEW.C:3913-3928 and DUNVIEW.C:8488-8533 preserve the D1L/D1R "
           "before D1C draw order for the C127 champion portrait sensor; "
           "CHAMPION.C F0297/F0298/F0302:243-285,662-706 are leader-hand "
           "put/remove/slot routes not entered by C161; DEFS.H:4041-4042 "
           "C160/C161/C162 names resurrect/reincarnate/cancel.";
}
