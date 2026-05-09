/*
 * CTest gate for DM1 V1 Resurrection & Reincarnation System.
 *
 * Tests all source-locked functions against ReDMCSB-derived invariants:
 *   F0860 bones creation, F0861 Vi Altar trigger,
 *   F0862 champion index extraction, F0863 rebirth health,
 *   F0864 reincarnation stat changes, F0865 command validation,
 *   F0866 C080/C127 candidate route, F0867 candidate panel finalization.
 */

#include <stdio.h>
#include <string.h>
#include "dm1_v1_resurrection_pc34_compat.h"

static int test_count = 0;
static int pass_count = 0;

#define CHECK(cond, msg) do { \
    test_count++; \
    if (cond) { pass_count++; } \
    else { printf("  FAIL: %s\n", msg); } \
} while(0)

static void test_bones_creation(void) {
    BonesCreationResult_Compat b;

    printf("[bones_creation]\n");

    /* Champion 0 at cell 2 */
    b = F0860_RESURRECTION_ComputeBonesCreation_Compat(0, 2);
    CHECK(b.junkType == 5, "junkType == C05_JUNK_BONES");
    CHECK(b.doNotDiscard == 1, "doNotDiscard == TRUE");
    CHECK(b.chargeCount == 0, "chargeCount == championIndex 0");
    CHECK(b.cell == 2, "cell matches champion cell");
    CHECK(b.valid == 1, "valid flag set");

    /* Champion 3 at cell 1 */
    b = F0860_RESURRECTION_ComputeBonesCreation_Compat(3, 1);
    CHECK(b.chargeCount == 3, "chargeCount == championIndex 3");
    CHECK(b.cell == 1, "cell 1");
}

static void test_vi_altar_trigger(void) {
    printf("[vi_altar_trigger]\n");

    /* All conditions met */
    CHECK(F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 1, 147) == 1,
        "alcove+viAltar+bones → trigger");

    /* Missing alcove */
    CHECK(F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(0, 1, 147) == 0,
        "no alcove → no trigger");

    /* Not facing Vi Altar */
    CHECK(F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 0, 147) == 0,
        "no viAltar → no trigger");

    /* Wrong item */
    CHECK(F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 1, 32) == 0,
        "wrong icon → no trigger");

    /* Non-bones junk item */
    CHECK(F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 1, 128) == 0,
        "boulder icon → no trigger");
}

static void test_champion_index_from_bones(void) {
    printf("[champion_index_from_bones]\n");

    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(0) == 0, "bones chargeCount 0 → champion 0");
    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(1) == 1, "bones chargeCount 1 → champion 1");
    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(2) == 2, "bones chargeCount 2 → champion 2");
    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(3) == 3, "bones chargeCount 3 → champion 3");
    /* Mask to 2 bits */
    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(7) == 3, "chargeCount 7 masked to 3");
}

static void test_rebirth_health(void) {
    RebirthHealthResult_Compat h;

    printf("[rebirth_health]\n");

    /* Normal case: 100 → max(25, 100-1-1) = 98, current = 49 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(100);
    CHECK(h.newMaxHealth == 98, "maxHealth 100 → 98");
    CHECK(h.newCurrentHealth == 49, "currentHealth → 49");

    /* Large health: 999 → max(25, 999 - 999/64 - 1) = max(25, 999-15-1) = 983 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(999);
    CHECK(h.newMaxHealth == 983, "maxHealth 999 → 983");
    CHECK(h.newCurrentHealth == 491, "currentHealth → 491");

    /* Near floor: 26 → max(25, 26-0-1) = 25, current = 12 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(26);
    CHECK(h.newMaxHealth == 25, "maxHealth 26 → 25");
    CHECK(h.newCurrentHealth == 12, "currentHealth → 12");

    /* At floor: 25 → max(25, 25-0-1) = 25, current = 12 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(25);
    CHECK(h.newMaxHealth == 25, "maxHealth 25 → 25");
    CHECK(h.newCurrentHealth == 12, "currentHealth → 12");

    /* Below floor: 10 → max(25, 10-0-1) = 25, current = 12 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(10);
    CHECK(h.newMaxHealth == 25, "maxHealth 10 → 25 (floor)");

    /* Repeated rebirth: 98 → max(25, 98-1-1) = 96, current = 48 */
    h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(98);
    CHECK(h.newMaxHealth == 96, "repeated rebirth: 98 → 96");
}

static void test_reincarnation(void) {
    ReincarnationResult_Compat r;
    uint8_t rng_all_zero[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t rng_spread[12]   = {0,1,2,3,4,5,6,0,1,2,3,4};
    int i, total;

    printf("[reincarnation]\n");

    /* Halving test */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        200, 180, 500, 400, 100, 80, rng_all_zero);
    CHECK(r.newMaxHealth == 100, "maxHealth halved: 200→100");
    CHECK(r.newCurrentHealth == 90, "currentHealth halved: 180→90");
    CHECK(r.newMaxStamina == 250, "maxStamina halved: 500→250");
    CHECK(r.newCurrentStamina == 200, "currentStamina halved: 400→200");
    CHECK(r.newMaxMana == 50, "maxMana halved: 100→50");
    CHECK(r.newCurrentMana == 40, "currentMana halved: 80→40");

    /* All 12 increments go to stat 0 */
    CHECK(r.statIncrements[0] == 12, "all rng→0: stat[0] gets 12 increments");
    CHECK(r.statIncrements[1] == 0, "stat[1] gets 0");

    /* Spread rng across stats */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        200, 180, 500, 400, 100, 80, rng_spread);
    total = 0;
    for (i = 0; i < 7; i++) total += r.statIncrements[i];
    CHECK(total == 12, "total increments == 12");
    CHECK(r.statIncrements[0] == 2, "stat[0] gets 2 (rng 0,0)");
    CHECK(r.statIncrements[1] == 2, "stat[1] gets 2 (rng 1,1)");
    CHECK(r.statIncrements[4] == 2, "stat[4] gets 2 (rng 4,4)");

    /* Odd health halving (integer truncation) */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        101, 51, 301, 151, 51, 25, rng_all_zero);
    CHECK(r.newMaxHealth == 50, "odd maxHealth 101→50 (truncated)");
    CHECK(r.newCurrentHealth == 25, "odd currentHealth 51→25");
}


static ChampionPortraitClickInput_Compat base_portrait_click_input(void) {
    ChampionPortraitClickInput_Compat in;
    in.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    in.leaderEmptyHanded = 1;
    in.leaderIndex = DM1_CHAMPION_NONE;
    in.frontWallOrnamentHit = 1;
    in.facingAlcove = 0;
    in.frontSquareInBounds = 1;
    in.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    in.sensorData = 11;
    in.sensorCell = 2;
    in.clickedWallCell = 2;
    in.partyChampionCount = 0;
    return in;
}

static void test_champion_portrait_candidate_route(void) {
    ChampionPortraitClickInput_Compat in;
    CandidateChampionAddResult_Compat r;

    printf("[champion_portrait_candidate_route]\n");

    in = base_portrait_click_input();
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 1, "C080+C05+C127 portrait route reaches F0280");
    CHECK(r.championPortraitIndex == 11, "sensor data is F0280 portrait index");
    CHECK(r.candidateChampionIndex == 0, "candidate inserted at previous party count");
    CHECK(r.candidateChampionOrdinal == 1, "candidate ordinal = previous count + 1");
    CHECK(r.nextPartyChampionCount == 1, "party count increments before panel decision");
    CHECK(r.setLeaderToFirstChampion == 1, "first candidate sets leader to champion 0");

    in = base_portrait_click_input();
    in.partyChampionCount = 3;
    in.sensorData = 5;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 1, "fourth slot candidate is allowed");
    CHECK(r.candidateChampionIndex == 3, "fourth candidate index is 3");
    CHECK(r.candidateChampionOrdinal == 4, "fourth candidate ordinal is 4");
    CHECK(r.nextPartyChampionCount == 4, "party can reach four champions");
    CHECK(r.setLeaderToFirstChampion == 0, "non-first candidate does not reset leader");

    in = base_portrait_click_input();
    in.leaderIndex = 0;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 1, "C127 route also works with existing leader");

    in = base_portrait_click_input();
    in.command = 7;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "non-C080 command cannot recruit");

    in = base_portrait_click_input();
    in.leaderEmptyHanded = 0;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "leader hand must be empty before F0280");

    in = base_portrait_click_input();
    in.frontWallOrnamentHit = 0;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "must hit front wall ornament C05");

    in = base_portrait_click_input();
    in.facingAlcove = 1;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "facing alcove blocks F0372 wall sensor touch");

    in = base_portrait_click_input();
    in.sensorType = 1;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "ordinary wall ornament sensor is not champion recruit");

    in = base_portrait_click_input();
    in.sensorCell = 1;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "sensor cell must equal clicked opposite wall cell");

    in = base_portrait_click_input();
    in.partyChampionCount = 4;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 0, "full party blocks F0280 candidate add");

    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(NULL);
    CHECK(r.triggersCandidateAdd == 0, "NULL input is safe no-op");
}

static void test_candidate_panel_path(void) {
    CandidatePanelState_Compat st;
    CandidatePanelResult_Compat r;

    printf("[candidate_panel_path]\n");

    st.partyChampionCount = 1;
    st.candidateChampionOrdinal = 1;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_CANCEL);
    CHECK(r.valid == 1, "cancel valid with candidate state");
    CHECK(r.cancelled == 1, "cancel flag set");
    CHECK(r.nextPartyChampionCount == 0, "cancel removes candidate from party count");
    CHECK(r.nextCandidateChampionOrdinal == 0, "cancel clears candidate ordinal");
    CHECK(r.disablesMirrorSensor == 0, "cancel does not disable mirror sensor");

    st.partyChampionCount = 2;
    st.candidateChampionOrdinal = 2;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_RESURRECT);
    CHECK(r.valid == 1, "resurrect valid with candidate state");
    CHECK(r.resurrected == 1, "resurrect flag set");
    CHECK(r.candidateChampionIndex == 1, "panel operates on G0305-1 champion");
    CHECK(r.nextPartyChampionCount == 2, "resurrect keeps candidate in party");
    CHECK(r.nextCandidateChampionOrdinal == 0, "resurrect clears candidate ordinal");
    CHECK(r.disablesMirrorSensor == 1, "resurrect disables mirror sensor");

    st.partyChampionCount = 4;
    st.candidateChampionOrdinal = 4;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_REINCARNATE);
    CHECK(r.valid == 1, "reincarnate valid with candidate state");
    CHECK(r.reincarnated == 1, "reincarnate flag set");
    CHECK(r.candidateChampionIndex == 3, "fourth candidate index is 3");
    CHECK(r.disablesMirrorSensor == 1, "reincarnate disables mirror sensor");

    st.partyChampionCount = 1;
    st.candidateChampionOrdinal = 0;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_RESURRECT);
    CHECK(r.valid == 0, "panel command blocked without prior candidate state");

    st.partyChampionCount = 2;
    st.candidateChampionOrdinal = 1;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_RESURRECT);
    CHECK(r.valid == 0, "candidate ordinal must match appended champion ordinal");

    st.partyChampionCount = 0;
    st.candidateChampionOrdinal = 0;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, DM1_COMMAND_CANCEL);
    CHECK(r.valid == 0, "empty party/candidate state is invalid");

    st.partyChampionCount = 1;
    st.candidateChampionOrdinal = 1;
    r = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(st, 163);
    CHECK(r.valid == 0, "unknown panel command invalid");
}

static void test_command_validation(void) {
    printf("[command_validation]\n");

    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(160, 1) == 1, "resurrect with 1 champ → valid");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(161, 2) == 1, "reincarnate with 2 champs → valid");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(162, 4) == 1, "cancel with 4 champs → valid");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(160, 0) == 0, "resurrect with 0 champs → invalid");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(99, 1) == 0, "bad command 99 → invalid");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(163, 1) == 0, "bad command 163 → invalid");
}

static void test_invariant(void) {
    printf("[invariant]\n");
    CHECK(dm1_v1_resurrection_GetInvariant() == 1u, "self-test invariant passes");
    CHECK(dm1_v1_resurrection_GetEvidence() != NULL, "evidence string non-null");
}

int main(void) {
    printf("probe=firestaff_dm1_v1_resurrection\n");

    test_bones_creation();
    test_vi_altar_trigger();
    test_champion_index_from_bones();
    test_rebirth_health();
    test_reincarnation();
    test_champion_portrait_candidate_route();
    test_candidate_panel_path();
    test_command_validation();
    test_invariant();

    printf("\nResults: %d/%d passed\n", pass_count, test_count);
    printf("sourceEvidence=%s\n", dm1_v1_resurrection_GetEvidence());
    printf("resurrectionInvariantOk=%u\n", dm1_v1_resurrection_GetInvariant());

    return (pass_count == test_count) ? 0 : 1;
}
