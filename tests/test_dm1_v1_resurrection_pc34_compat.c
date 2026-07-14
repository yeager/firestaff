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
    in.sensorData = 23;
    r = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
    CHECK(r.triggersCandidateAdd == 1, "last C026 atlas portrait index is accepted");
    CHECK(r.championPortraitIndex == 23, "C127 sensorData is a 0..23 portrait atlas index");

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

static void test_candidate_append_clear_cycles(void) {
    ChampionPortraitClickInput_Compat in;
    CandidateChampionAddResult_Compat add;
    CandidatePanelState_Compat st;
    CandidatePanelResult_Compat clear;
    int cycle;

    printf("[candidate_append_clear_cycles]\n");

    /* ReDMCSB REVIVE.C:272-276 / F0280 sets G0299 to
     * previousPartyChampionCount + 1 and increments G0305.  REVIVE.C:744-783
     * / F0282 cancel clears G0299 and decrements G0305 without taking the
     * REVIVE.C:785-799 mirror-sensor disable path.  Repeating the route
     * must therefore reuse the same appended party slot until a non-cancel
     * command finalizes the candidate. */
    in = base_portrait_click_input();
    for (cycle = 0; cycle < 3; ++cycle) {
        in.partyChampionCount = 0;
        add = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
        CHECK(add.triggersCandidateAdd == 1, "cycle F0280 route remains armed");
        CHECK(add.candidateChampionIndex == 0, "cycle candidate index resets to slot 0");
        CHECK(add.candidateChampionOrdinal == 1, "cycle G0299 ordinal resets to 1");
        CHECK(add.nextPartyChampionCount == 1, "cycle G0305 increments to 1");

        st.partyChampionCount = add.nextPartyChampionCount;
        st.candidateChampionOrdinal = add.candidateChampionOrdinal;
        clear = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(
            st, DM1_COMMAND_CANCEL);
        CHECK(clear.valid == 1, "cycle cancel is valid while G0299 is live");
        CHECK(clear.cancelled == 1, "cycle cancel flag set");
        CHECK(clear.nextPartyChampionCount == 0, "cycle cancel decrements G0305");
        CHECK(clear.nextCandidateChampionOrdinal == 0, "cycle cancel clears G0299");
        CHECK(clear.disablesMirrorSensor == 0, "cycle cancel does not disable mirror sensor");
    }
}

static void test_mirror_sensor_disable_order(void) {
    MirrorThing_Compat things[3];
    MirrorSensorDisableResult_Compat d;

    printf("[mirror_sensor_disable_order]\n");

    things[0].thingType = DM1_THING_TYPE_TEXTSTRING;
    things[0].sensorType = 999;
    things[1].thingType = DM1_THING_TYPE_SENSOR;
    things[1].sensorType = 12;
    things[2].thingType = DM1_THING_TYPE_SENSOR;
    things[2].sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    d = F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(things, 3);
    CHECK(d.foundSensor == 1, "helper finds first C03 sensor thing");
    CHECK(d.disabledThingIndex == 1, "textstring before sensor is skipped");
    CHECK(d.disabledOldSensorType == 12, "first sensor type is disabled even when not C127");
    CHECK(d.disabledNewSensorType == DM1_SENSOR_DISABLED, "disabled type becomes C000");

    things[0].thingType = DM1_THING_TYPE_SENSOR;
    things[0].sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    things[1].thingType = DM1_THING_TYPE_SENSOR;
    things[1].sensorType = 12;
    d = F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(things, 2);
    CHECK(d.disabledThingIndex == 0, "C127 is disabled only when it is first sensor in thing-list order");
    CHECK(d.disabledOldSensorType == DM1_SENSOR_WALL_CHAMPION_PORTRAIT, "C127 preserved as old type before disable");

    things[0].thingType = DM1_THING_TYPE_TEXTSTRING;
    things[1].thingType = DM1_THING_TYPE_TEXTSTRING;
    d = F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(things, 2);
    CHECK(d.foundSensor == 0, "bounded helper reports no sensor in custom malformed input");
    CHECK(d.disabledThingIndex == -1, "no sensor leaves index at -1");

    d = F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(NULL, 2);
    CHECK(d.foundSensor == 0, "NULL thing list is safe no-op for probe helper");
}

static ViAltarFullCycleInput_Compat base_vi_altar_full_cycle_input(void) {
    ViAltarFullCycleInput_Compat in;
    in.championIndex = 2;
    in.oldChampionCell = 1;
    in.occupiedCellMask = 0x0Bu;
    in.partyDirection = 3;
    in.maximumHealth = 100;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 2;
    in.bonesCell = 3;
    return in;
}

static void test_vi_altar_full_cycle_transition(void) {
    ViAltarFullCycleInput_Compat in;
    ViAltarFullCycleResult_Compat r;

    printf("[vi_altar_full_cycle_transition]\n");

    /* ReDMCSB source chain for this exact transition:
     *   CLIKVIEW.C:F0374:173-186 writes C13_EVENT_VI_ALTAR_REBIRTH with
     *     Effect=C02_EFFECT_TOGGLE and Priority=JUNK.ChargeCount.
     *   TIMELINE.C:1665-1698 processes step 2 (C0xFFE4 rebirth explosion,
     *     +5 ticks), step 1 (matching bones icon/ChargeCount unlinked), then
     *     step 0 (F0283_CHAMPION_ViAltarRebirth(priority)).
     *   REVIVE.C:F0283:915-937 relocates an occupied old cell to the first
     *     free cell, applies max/current health, copies party direction, and
     *     marks MASK0x8000_ACTION_HAND | MASK0x1000_STATUS_BOX | MASK0x0400_ICON. */
    in = base_vi_altar_full_cycle_input();
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 1, "F0374 creates a Vi altar rebirth event for alcove+Vi+bones");
    CHECK(r.eventType == DM1_EVENT_TYPE_VI_ALTAR_REBIRTH, "event type is C13_EVENT_VI_ALTAR_REBIRTH");
    CHECK(r.eventPriority == 2, "event priority is bones ChargeCount champion candidate 2");
    CHECK(r.eventEffect == DM1_EFFECT_TOGGLE, "initial rebirth event effect is step 2");
    CHECK(r.step2ExplosionThing == DM1_EXPLOSION_REBIRTH_STEP1, "timeline step 2 creates C0xFFE4 rebirth explosion");
    CHECK(r.step2ExplosionType == DM1_EXPLOSION_TYPE_REBIRTH_STEP1, "C0xFFE4 maps to C100 rebirth step 1 aspect");
    CHECK(r.step2DelayTicks == 5, "step 2 delays the next event by 5 ticks");
    CHECK(r.step1BonesMatched == 1, "step 1 finds matching bones by icon/cell/ChargeCount");
    CHECK(r.step1BonesUnlinked == 1, "step 1 unlinks matching bones before revive");
    CHECK(r.revived == 1, "step 0 reaches F0283 champion rebirth");
    CHECK(r.championIndex == 2, "reborn champion index remains the candidate from bones");
    CHECK(r.finalCell == 2, "old occupied cell 1 relocates to first free cell 2");
    CHECK(r.finalMaximumHealth == 98, "maximum health 100 gets Vi altar penalty to 98");
    CHECK(r.finalCurrentHealth == 49, "current health becomes half of penalized maximum");
    CHECK(r.finalDirection == 3, "reborn champion direction copies party direction");
    CHECK(r.dirtyAttributes ==
              (DM1_CHAMPION_ATTR_ACTION_HAND |
               DM1_CHAMPION_ATTR_STATUS_BOX |
               DM1_CHAMPION_ATTR_ICON),
          "rebirth marks action-hand/status/icon redraw attributes");

    in = base_vi_altar_full_cycle_input();
    in.bonesChargeCount = 1;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 1, "mismatched bones still schedule the event from F0374");
    CHECK(r.step1BonesMatched == 0, "step 1 rejects bones whose ChargeCount is not the candidate");
    CHECK(r.step1BonesUnlinked == 0, "mismatched bones are not unlinked");
    CHECK(r.revived == 0, "mismatched bones do not reach F0283 rebirth");

    in = base_vi_altar_full_cycle_input();
    in.facingViAltar = 0;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 0, "non-Vi alcove drop does not create the rebirth event");
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
    test_candidate_append_clear_cycles();
    test_mirror_sensor_disable_order();
    test_vi_altar_full_cycle_transition();
    test_command_validation();
    test_invariant();

    printf("\nResults: %d/%d passed\n", pass_count, test_count);
    printf("sourceEvidence=%s\n", dm1_v1_resurrection_GetEvidence());
    printf("resurrectionInvariantOk=%u\n", dm1_v1_resurrection_GetInvariant());

    return (pass_count == test_count) ? 0 : 1;
}
