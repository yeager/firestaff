#include "dm1_v1_mirror_candidate_resurrect_reincarnate_skills_pc34_compat.h"

#include <string.h>

enum {
    C1_STATISTIC_STRENGTH_PC34_COMPAT = 1,
    C6_STATISTIC_ANTIFIRE_PC34_COMPAT = 6,
    C1_CURRENT_PC34_COMPAT = 1,
    C0_MAXIMUM_PC34_COMPAT = 0,
    C2_MINIMUM_PC34_COMPAT = 2,
    THING_NONE_PC34_COMPAT = -1
};

static Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat
    gAfterC160;
static Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat
    gAfterC161;

static void copy_before_to_after(
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state)
{
    memcpy(state->after, state->before, sizeof(state->after));
}

void DM1_V1_MirrorCandidateResurrectReincarnateSkills_InitPc34Compat(
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;
    int skillIndex;
    int statisticIndex;
    Dm1V1MirrorCandidateResurrectReincarnateSkillsChampionPc34Compat *candidate;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->candidateChampionIndex = 1;
    state->partyChampionCountBefore = 2;
    state->partyChampionCountAfter = 2;
    state->candidateChampionOrdinalBefore = 2u;
    state->candidateChampionOrdinalAfter = 2u;
    state->g0411CandidateIdentity = 1;
    state->c040PanelOpenBefore = 1;
    state->c040PanelOpenAfter = 1;
    state->leaderIndexBefore = 0;
    state->leaderIndexAfter = 0;
    state->leaderHandThingBefore = THING_NONE_PC34_COMPAT;
    state->leaderHandThingAfter = THING_NONE_PC34_COMPAT;
    state->mapXAfterDirectionStep = 12;
    state->mapYAfterDirectionStep = 21;

    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        state->before[championIndex].present = championIndex < 2;
        state->before[championIndex].alive = championIndex == 0;
        for (slotIndex = 0;
             slotIndex <
                 DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            state->before[championIndex].slots[slotIndex] =
                championIndex == 1 ? 1000 + slotIndex : THING_NONE_PC34_COMPAT;
        }
    }

    state->before[0].currentHealth = 48;
    state->before[0].maximumHealth = 96;
    state->before[0].currentStamina = 70;
    state->before[0].maximumStamina = 140;
    state->before[0].currentMana = 22;
    state->before[0].maximumMana = 44;

    candidate = &state->before[1];
    candidate->currentHealth = 88;
    candidate->maximumHealth = 176;
    candidate->currentStamina = 72;
    candidate->maximumStamina = 144;
    candidate->currentMana = 40;
    candidate->maximumMana = 80;
    for (skillIndex = 0;
         skillIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SKILL_BYTE_COUNT_PC34_COMPAT;
         ++skillIndex) {
        candidate->skills[skillIndex] = (unsigned char)(0x40 + skillIndex);
    }
    for (statisticIndex = 0;
         statisticIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_STATISTIC_COUNT_PC34_COMPAT;
         ++statisticIndex) {
        candidate->statistics[statisticIndex][C0_MAXIMUM_PC34_COMPAT] =
            48 + statisticIndex * 8;
        candidate->statistics[statisticIndex][C1_CURRENT_PC34_COMPAT] =
            48 + statisticIndex * 8;
        candidate->statistics[statisticIndex][C2_MINIMUM_PC34_COMPAT] = 30;
    }
    copy_before_to_after(state);
}

static void apply_shared_c160_c161_tail(
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state)
{
    int slotIndex;

    /* ReDMCSB: REVIVE.C F0282_CHAMPION_ProcessCommands160To162_
     * ClickInResurrectReincarnatePanel:704-806 dispatches C160/C161/C162.
     * The live C160/C161 tail at lines 785-790 clears G0299 and computes
     * L0828/L0829 as party map plus direction step; lines 790-794 call
     * F0164_DUNGEON_UnlinkThingFromList for each non-empty C00..C29 slot;
     * lines 794-806 implement BUG0_87 by disabling the first sensor in the
     * mirror-square thing list. */
    state->candidateChampionOrdinalAfter = 0u;
    state->c040PanelOpenAfter = 0;
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        if (state->before[1].slots[slotIndex] != THING_NONE_PC34_COMPAT) {
            state->f0164UnlinkedSlots[slotIndex] = 1;
            ++state->f0164UnlinkCallCount;
        }
    }
    state->bug087SensorWalkCount = 2;
    state->bug087FirstSensorDisabled = 1;

    /* ReDMCSB: REVIVE.C F0282:758-783 describes the I34 cancel cleanup draw
     * calls and the same status/icon surfaces that must remain refreshed when
     * this contract mirrors C160/C161 finalization: M524_FillScreenBox,
     * F0733_FillZoneByIndex, F0621_ClearChampionIconBox,
     * F0457_START_DrawEnabledMenus_CPSF, and F0078_MOUSE_DisableScreenUpdate.
     * F0368_COMMAND_SetLeader and F0394_MENUS_SetMagicCasterAndDrawSpellArea
     * are count==1-only there; this fixture keeps party count > 1. */
    state->m524FillChampionStatusBoxCallCount = 1;
    state->f0733FillStatusZoneCallCount = 1;
    state->f0621ClearIconBoxCallCount = 1;
    state->f0457DrawEnabledMenusCallCount = 1;
    state->f0078DisableScreenUpdateCallCount = 1;
    state->f0368SetLeaderCallCount = 0;
    state->f0394SetMagicCasterCallCount = 0;
    state->leaderIndexAfter = state->leaderIndexBefore;
    state->leaderHandThingAfter = state->leaderHandThingBefore;

    /* ReDMCSB: CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,
     * 511-515,606-610,688-710 are leader-hand and slot-click paths. The
     * C040 C160/C161 panel command must not re-enter them. */
    state->f0297LeaderHandPutRouteCount = 0;
    state->f0298LeaderHandRemoveRouteCount = 0;
    state->f0300SlotRemoveRouteCount = 0;
    state->f0301SlotAddRouteCount = 0;
    state->f0302SlotBoxRouteCount = 0;
}

static void drive_command(
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state,
    int command)
{
    int statisticIndex;
    int current;
    int oneEighth;

    DM1_V1_MirrorCandidateResurrectReincarnateSkills_InitPc34Compat(state);
    state->command = command;
    state->validPanelCommand =
        command == DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT ||
        command == DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT;
    if (!state->validPanelCommand) {
        return;
    }

    apply_shared_c160_c161_tail(state);
    state->after[1].alive = 1;
    state->f0281RenameCallCount = 1;

    if (command !=
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT) {
        return;
    }

    /* ReDMCSB: REVIVE.C F0282:808-810 is the C161-only
     * F0281_CHAMPION_Rename plus F0008_MAIN_ClearBytes on Skills path.
     * REVIVE.C F0282:810-820 applies CHANGE7_24_IMPROVEMENT: C1..C6 lose
     * current/8 without going below minimum, and health/stamina/mana current
     * and maximum values are halved. */
    state->f0008ClearSkillsCallCount = 1;
    memset(state->after[1].skills, 0, sizeof(state->after[1].skills));
    for (statisticIndex = C1_STATISTIC_STRENGTH_PC34_COMPAT;
         statisticIndex <= C6_STATISTIC_ANTIFIRE_PC34_COMPAT;
         ++statisticIndex) {
        current =
            state->after[1].statistics[statisticIndex][C1_CURRENT_PC34_COMPAT];
        oneEighth = current >> 3;
        state->after[1].statistics[statisticIndex][C1_CURRENT_PC34_COMPAT] =
            current - oneEighth;
        state->after[1].statistics[statisticIndex][C0_MAXIMUM_PC34_COMPAT] =
            current - oneEighth;
    }
    state->after[1].currentHealth >>= 1;
    state->after[1].maximumHealth >>= 1;
    state->after[1].currentStamina >>= 1;
    state->after[1].maximumStamina >>= 1;
    state->after[1].currentMana >>= 1;
    state->after[1].maximumMana >>= 1;
}

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC160Pc34Compat(void)
{
    drive_command(&gAfterC160,
                  DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT);
    return &gAfterC160;
}

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC161Pc34Compat(void)
{
    drive_command(&gAfterC161,
                  DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT);
    return &gAfterC161;
}

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_StateAfterPc34Compat(
    int command)
{
    if (command ==
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT) {
        return &gAfterC160;
    }
    if (command ==
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT) {
        return &gAfterC161;
    }
    return 0;
}

const char *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB REVIVE.C F0282_CHAMPION_ProcessCommands160To162_"
           "ClickInResurrectReincarnatePanel:704-806 dispatches C160/C161/"
           "C162; REVIVE.C F0282:758-783 is the C162 cancel status/icon draw "
           "cleanup and count==1-only F0368/F0394 path; REVIVE.C F0282:"
           "785-790 clears G0299 and computes L0828/L0829 as party plus "
           "direction step; REVIVE.C F0282:790-794 unlinks non-empty C00..C29 "
           "slots with F0164_DUNGEON_UnlinkThingFromList; REVIVE.C F0282:"
           "794-806 is BUG0_87 first-sensor disablement; REVIVE.C F0282:"
           "808-810 is C161-only F0281_CHAMPION_Rename and "
           "F0008_MAIN_ClearBytes on Skills; REVIVE.C F0282:810-820 is "
           "C161-only CHANGE7_24_IMPROVEMENT statistic and resource halving; "
           "CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,511-515,"
           "606-610,688-710 are leader-hand identity and slot-clear paths "
           "that C160/C161 do not re-enter.";
}
