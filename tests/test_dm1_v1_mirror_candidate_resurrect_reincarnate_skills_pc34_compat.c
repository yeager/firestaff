#include "dm1_v1_mirror_candidate_resurrect_reincarnate_skills_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
        printf("PASS: %s [%s]\n", msg, anchor); \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void check_initial_fixture(void)
{
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat state;
    int slotIndex;

    DM1_V1_MirrorCandidateResurrectReincarnateSkills_InitPc34Compat(&state);
    CHECK_REDMCSB(state.partyChampionCountBefore == 2,
                  "fixture has champion0 plus candidate1 in the party",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state.before[0].alive == 1 && state.before[1].alive == 0,
                  "fixture models champion0 alive and candidate1 dead",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state.c040PanelOpenBefore == 1 &&
                      state.candidateChampionOrdinalBefore == 2u,
                  "C040 panel starts open with G0299 on candidate1 ordinal",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state.g0411CandidateIdentity == 1,
                  "G0411-style candidate identity points to candidate1",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state.leaderHandThingBefore ==
                      DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_NONE_PC34_COMPAT,
                  "leader hand starts empty before C160/C161",
                  "CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,"
                  "511-515,606-610,688-710");
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        CHECK_REDMCSB(state.before[1].slots[slotIndex] == 1000 + slotIndex,
                      "candidate1 C00..C29 slot fixture is non-empty",
                      "REVIVE.C F0282:790-794");
    }
}

static void check_shared_finalization(
    const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state,
    const char *label)
{
    int slotIndex;

    CHECK_REDMCSB(state != NULL, label, "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->validPanelCommand == 1,
                  "C160/C161 driver accepted a live C040 panel command",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->candidateChampionOrdinalBefore == 2u &&
                      state->candidateChampionOrdinalAfter == 0u,
                  "G0299 is cleared after the live panel command",
                  "REVIVE.C F0282:785-790");
    CHECK_REDMCSB(state->mapXAfterDirectionStep == 12 &&
                      state->mapYAfterDirectionStep == 21,
                  "mirror-square map coordinate uses party plus direction step",
                  "REVIVE.C F0282:785-790");
    CHECK_REDMCSB(state->f0164UnlinkCallCount ==
                      DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT,
                  "F0164 slot-unlink loop runs across C00..C29",
                  "REVIVE.C F0282:790-794");
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        CHECK_REDMCSB(state->f0164UnlinkedSlots[slotIndex] == 1,
                      "individual non-empty candidate slot is unlinked",
                      "REVIVE.C F0282:790-794");
    }
    CHECK_REDMCSB(state->bug087SensorWalkCount == 2 &&
                      state->bug087FirstSensorDisabled == 1,
                  "BUG0_87 disables the first sensor encountered and breaks",
                  "REVIVE.C F0282:794-806");
    CHECK_REDMCSB(state->c040PanelOpenBefore == 1 &&
                      state->c040PanelOpenAfter == 0,
                  "C040 panel is closed after the finalization path",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->m524FillChampionStatusBoxCallCount == 1,
                  "champion-status box clear is represented",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0733FillStatusZoneCallCount == 1,
                  "I34 status-box zone clear is represented",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0621ClearIconBoxCallCount == 1,
                  "I34 champion-icon box clear is represented",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0457DrawEnabledMenusCallCount == 1,
                  "enabled menus are redrawn after panel finalization",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0078DisableScreenUpdateCallCount == 1,
                  "screen update is disabled again after redraw",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0368SetLeaderCallCount == 0,
                  "F0368 is not called because party count is greater than one",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->f0394SetMagicCasterCallCount == 0,
                  "F0394 is not called because party count is greater than one",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(state->leaderIndexBefore == state->leaderIndexAfter &&
                      state->leaderHandThingBefore == state->leaderHandThingAfter,
                  "leader identity and empty leader hand remain unchanged",
                  "CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,"
                  "511-515,606-610,688-710");
    CHECK_REDMCSB(state->f0297LeaderHandPutRouteCount == 0 &&
                      state->f0298LeaderHandRemoveRouteCount == 0,
                  "leader-hand put/remove paths are not re-entered",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB(state->f0300SlotRemoveRouteCount == 0 &&
                      state->f0301SlotAddRouteCount == 0 &&
                      state->f0302SlotBoxRouteCount == 0,
                  "slot remove/add/click paths are not re-entered",
                  "CHAMPION.C F0300/F0301/F0302:511-515,606-610,688-710");
    CHECK_REDMCSB(state->after[1].alive == 1,
                  "candidate1 is live after C160/C161 finalization",
                  "REVIVE.C F0282:704-806");
}

static void check_c160_distinction(void)
{
    const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state =
        DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC160Pc34Compat();
    const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *after =
        DM1_V1_MirrorCandidateResurrectReincarnateSkills_StateAfterPc34Compat(
            DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT);
    int skillIndex;
    int statisticIndex;

    check_shared_finalization(state, "C160 state is available");
    CHECK_REDMCSB(after == state,
                  "state_after(C160) returns the C160 driver result",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->command ==
                      DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT,
                  "C160 driver records resurrect command 160",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->f0281RenameCallCount == 1,
                  "C160 records the rename handoff required by this gate",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->f0008ClearSkillsCallCount == 0,
                  "C160 does not call F0008_MAIN_ClearBytes on Skills",
                  "REVIVE.C F0282:808-810");
    for (skillIndex = 0;
         skillIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SKILL_BYTE_COUNT_PC34_COMPAT;
         ++skillIndex) {
        CHECK_REDMCSB(state->after[1].skills[skillIndex] ==
                          state->before[1].skills[skillIndex],
                      "C160 preserves every candidate Skills byte",
                      "REVIVE.C F0282:808-810");
    }
    for (statisticIndex = 1; statisticIndex <= 6; ++statisticIndex) {
        CHECK_REDMCSB(
            state->after[1].statistics[statisticIndex][1] ==
                state->before[1].statistics[statisticIndex][1] &&
                state->after[1].statistics[statisticIndex][0] ==
                    state->before[1].statistics[statisticIndex][0],
            "C160 leaves C1..C6 current and maximum statistics unchanged",
            "REVIVE.C F0282:810-820");
    }
    CHECK_REDMCSB(state->after[1].currentHealth ==
                      state->before[1].currentHealth &&
                      state->after[1].maximumHealth ==
                          state->before[1].maximumHealth,
                  "C160 leaves health values unchanged",
                  "REVIVE.C F0282:810-820");
    CHECK_REDMCSB(state->after[1].currentStamina ==
                      state->before[1].currentStamina &&
                      state->after[1].maximumStamina ==
                          state->before[1].maximumStamina,
                  "C160 leaves stamina values unchanged",
                  "REVIVE.C F0282:810-820");
    CHECK_REDMCSB(state->after[1].currentMana ==
                      state->before[1].currentMana &&
                      state->after[1].maximumMana ==
                          state->before[1].maximumMana,
                  "C160 leaves mana values unchanged",
                  "REVIVE.C F0282:810-820");
}

static void check_c161_distinction(void)
{
    const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state =
        DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC161Pc34Compat();
    const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *after =
        DM1_V1_MirrorCandidateResurrectReincarnateSkills_StateAfterPc34Compat(
            DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT);
    int skillIndex;
    int statisticIndex;
    int beforeCurrent;
    int expected;

    check_shared_finalization(state, "C161 state is available");
    CHECK_REDMCSB(after == state,
                  "state_after(C161) returns the C161 driver result",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->command ==
                      DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT,
                  "C161 driver records reincarnate command 161",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(state->f0281RenameCallCount == 1,
                  "C161 calls F0281_CHAMPION_Rename",
                  "REVIVE.C F0282:808-810");
    CHECK_REDMCSB(state->f0008ClearSkillsCallCount == 1,
                  "C161 calls F0008_MAIN_ClearBytes on Skills",
                  "REVIVE.C F0282:808-810");
    for (skillIndex = 0;
         skillIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SKILL_BYTE_COUNT_PC34_COMPAT;
         ++skillIndex) {
        CHECK_REDMCSB(state->before[1].skills[skillIndex] != 0 &&
                          state->after[1].skills[skillIndex] == 0,
                      "C161 clears every candidate Skills byte",
                      "REVIVE.C F0282:808-810");
    }
    for (statisticIndex = 1; statisticIndex <= 6; ++statisticIndex) {
        beforeCurrent = state->before[1].statistics[statisticIndex][1];
        expected = beforeCurrent - (beforeCurrent >> 3);
        CHECK_REDMCSB(
            state->after[1].statistics[statisticIndex][1] == expected &&
                state->after[1].statistics[statisticIndex][0] == expected,
            "C161 CHANGE7_24 drops each C1..C6 statistic by current/8",
            "REVIVE.C F0282:810-820");
    }
    CHECK_REDMCSB(state->after[1].currentHealth ==
                      state->before[1].currentHealth / 2 &&
                      state->after[1].maximumHealth ==
                          state->before[1].maximumHealth / 2,
                  "C161 halves health current and maximum",
                  "REVIVE.C F0282:810-820");
    CHECK_REDMCSB(state->after[1].currentStamina ==
                      state->before[1].currentStamina / 2 &&
                      state->after[1].maximumStamina ==
                          state->before[1].maximumStamina / 2,
                  "C161 halves stamina current and maximum",
                  "REVIVE.C F0282:810-820");
    CHECK_REDMCSB(state->after[1].currentMana ==
                      state->before[1].currentMana / 2 &&
                      state->after[1].maximumMana ==
                          state->before[1].maximumMana / 2,
                  "C161 halves mana current and maximum",
                  "REVIVE.C F0282:810-820");
}

static void check_source_evidence(void)
{
    const char *evidence =
        DM1_V1_MirrorCandidateResurrectReincarnateSkills_SourceEvidencePc34Compat();

    CHECK_REDMCSB(evidence != NULL,
                  "source evidence string exists",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282_CHAMPION_ProcessCommands160To162_") != NULL,
                  "evidence cites F0282 function name",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(strstr(evidence, "704-806") != NULL,
                  "evidence cites C160/C161/C162 dispatch range",
                  "REVIVE.C F0282:704-806");
    CHECK_REDMCSB(strstr(evidence, "758-783") != NULL,
                  "evidence cites C162 status/icon cleanup range",
                  "REVIVE.C F0282:758-783");
    CHECK_REDMCSB(strstr(evidence, "785-790") != NULL,
                  "evidence cites post-cancel G0299/map step range",
                  "REVIVE.C F0282:785-790");
    CHECK_REDMCSB(strstr(evidence, "790-794") != NULL,
                  "evidence cites C00..C29 unlink loop range",
                  "REVIVE.C F0282:790-794");
    CHECK_REDMCSB(strstr(evidence, "794-806") != NULL,
                  "evidence cites BUG0_87 sensor disablement range",
                  "REVIVE.C F0282:794-806");
    CHECK_REDMCSB(strstr(evidence, "808-810") != NULL,
                  "evidence cites C161-only Skills clear range",
                  "REVIVE.C F0282:808-810");
    CHECK_REDMCSB(strstr(evidence, "810-820") != NULL,
                  "evidence cites CHANGE7_24 statistic loop range",
                  "REVIVE.C F0282:810-820");
    CHECK_REDMCSB(strstr(evidence, "CHAMPION.C F0297/F0298/F0300/F0301/F0302") != NULL,
                  "evidence cites leader-hand and slot-clear functions",
                  "CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,"
                  "511-515,606-610,688-710");
}

int main(void)
{
    check_initial_fixture();
    check_c160_distinction();
    check_c161_distinction();
    check_source_evidence();

    printf("PASS dm1_v1_mirror_candidate_resurrect_reincarnate_skills_pc34_compat "
           "assertionCount=%d passed=%d [REVIVE.C F0282:704-806]\n",
           gTests, gPasses);
    return gPasses == gTests ? 0 : 1;
}
