#ifndef DM1_V1_MIRROR_CANDIDATE_REINCARNATE_REARM_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_REARM_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_RESURRECT_COMMAND_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CANCEL_COMMAND_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_INVENTORY_TOGGLE_0_PC34_COMPAT 7
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_SPELL_AREA_COMMAND_PC34_COMPAT 100
#define DM1_V1_MIRROR_CANDIDATE_REINCARNATE_ACTION_AREA_COMMAND_PC34_COMPAT 111

typedef struct Dm1V1MirrorCandidateReincarnateChampionPc34Compat {
    int present;
    int currentHealth;
    int maximumHealth;
    int currentStamina;
    int maximumStamina;
    int currentMana;
    int maximumMana;
    int portraitOrdinal;
    int skillBytesNonZero;
} Dm1V1MirrorCandidateReincarnateChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    int frontD1cMirrorChampionOrdinal;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    int firstMirrorSensorDisabled;
    int leaderHandThingOrdinal;
    int leaderHandPutRouteCount;
    int leaderHandRemoveRouteCount;
    int slotBoxRouteCount;
    Dm1V1MirrorCandidateReincarnateChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_REINCARNATE_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat;

typedef struct Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat {
    int command;
    int validPanelCommand;
    int candidateChampionIndex;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    int candidateCleared;
    int c040PanelCleared;
    int mirrorSensorDisabled;
    int mirrorRouteRearmed;
    int resurrected;
    int reincarnated;
    int resurrectContractPreserved;
    int ignoredNoCandidate;
    int ignoredWrongCommand;
    int currentHealthBefore;
    int currentHealthAfter;
    int maximumHealthBefore;
    int maximumHealthAfter;
    int currentStaminaBefore;
    int currentStaminaAfter;
    int maximumStaminaBefore;
    int maximumStaminaAfter;
    int currentManaBefore;
    int currentManaAfter;
    int maximumManaBefore;
    int maximumManaAfter;
    int skillsCleared;
    int skillBytesNonZeroBefore;
    int skillBytesNonZeroAfter;
    int previousLeaderIndex;
    int newLeaderIndex;
    int previousLeaderHandThingOrdinal;
    int newLeaderHandThingOrdinal;
    int leaderHandPutRouteEntered;
    int leaderHandRemoveRouteEntered;
    int slotBoxRouteEntered;
    int previousFrontD1cMirrorChampionOrdinal;
    int newFrontD1cMirrorChampionOrdinal;
    int frontD1cPortraitIndex;
} Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat;

typedef struct Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat {
    int command;
    int panelC040Closed;
    int statusBoxAllowed;
    int inventoryToggleAllowed;
    int spellAreaAllowed;
    int actionAreaAllowed;
    int blockedByCandidatePanel;
} Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat;

void DM1_V1_MirrorCandidateReincarnateRearm_InitPc34Compat(
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state);

int DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
    const Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat *outResult);

const char *DM1_V1_MirrorCandidateReincarnateRearm_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
