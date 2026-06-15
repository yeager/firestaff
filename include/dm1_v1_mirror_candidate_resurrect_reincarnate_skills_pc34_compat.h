#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_REINCARNATE_SKILLS_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_REINCARNATE_SKILLS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C160_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C161_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_COMMAND_C162_PC34_COMPAT 162

#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SKILL_BYTE_COUNT_PC34_COMPAT 24
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_STATISTIC_COUNT_PC34_COMPAT 7
#define DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_NONE_PC34_COMPAT (-1)

typedef struct Dm1V1MirrorCandidateResurrectReincarnateSkillsChampionPc34Compat {
    int present;
    int alive;
    int currentHealth;
    int maximumHealth;
    int currentStamina;
    int maximumStamina;
    int currentMana;
    int maximumMana;
    unsigned char skills[
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SKILL_BYTE_COUNT_PC34_COMPAT];
    int statistics[
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_STATISTIC_COUNT_PC34_COMPAT][3];
    int slots[DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateResurrectReincarnateSkillsChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat {
    int command;
    int validPanelCommand;
    int partyChampionCountBefore;
    int partyChampionCountAfter;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    int candidateChampionIndex;
    int g0411CandidateIdentity;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leaderHandThingBefore;
    int leaderHandThingAfter;
    int mapXAfterDirectionStep;
    int mapYAfterDirectionStep;
    int f0281RenameCallCount;
    int f0008ClearSkillsCallCount;
    int f0164UnlinkCallCount;
    int f0164UnlinkedSlots[
        DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_SLOT_COUNT_PC34_COMPAT];
    int bug087SensorWalkCount;
    int bug087FirstSensorDisabled;
    int f0368SetLeaderCallCount;
    int f0394SetMagicCasterCallCount;
    int m524FillChampionStatusBoxCallCount;
    int f0733FillStatusZoneCallCount;
    int f0621ClearIconBoxCallCount;
    int f0457DrawEnabledMenusCallCount;
    int f0078DisableScreenUpdateCallCount;
    int f0297LeaderHandPutRouteCount;
    int f0298LeaderHandRemoveRouteCount;
    int f0300SlotRemoveRouteCount;
    int f0301SlotAddRouteCount;
    int f0302SlotBoxRouteCount;
    Dm1V1MirrorCandidateResurrectReincarnateSkillsChampionPc34Compat
        before[DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_CHAMPION_COUNT_PC34_COMPAT];
    Dm1V1MirrorCandidateResurrectReincarnateSkillsChampionPc34Compat
        after[DM1_V1_MIRROR_CANDIDATE_RR_SKILLS_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat;

void DM1_V1_MirrorCandidateResurrectReincarnateSkills_InitPc34Compat(
    Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *state);

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC160Pc34Compat(void);

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_DriveC161Pc34Compat(void);

const Dm1V1MirrorCandidateResurrectReincarnateSkillsStatePc34Compat *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_StateAfterPc34Compat(
    int command);

const char *
DM1_V1_MirrorCandidateResurrectReincarnateSkills_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
