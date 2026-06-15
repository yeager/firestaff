#ifndef DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H
#define DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT 4
#define DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT (-1)

#define DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT 0x0002u

#define DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_COMMAND_CLICK_STATUS_BOX_3_PC34_COMPAT 15
#define DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT 16
#define DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT 19
#define DM1_V1_COMMAND_NONE_PC34_COMPAT 0

typedef struct Dm1V1ChampionMirrorChampionPc34Compat {
    int currentHealth;
} Dm1V1ChampionMirrorChampionPc34Compat;

typedef struct Dm1V1ChampionMirrorClickStatePc34Compat {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    Dm1V1ChampionMirrorChampionPc34Compat
        champions[DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT];
} Dm1V1ChampionMirrorClickStatePc34Compat;

typedef struct Dm1V1ChampionMirrorClickResultPc34Compat {
    int previousLeaderIndex;
    int newLeaderIndex;
    int clickedChampionIndex;
    int nestedCommand;
    int targetLeaderIndex;
    int dispatchesStatusBoxClick;
    int ignoredByCandidatePanel;
    int ignoredOutOfParty;
    int attemptedSetLeader;
    int leaderChanged;
    int ignoredSameLeader;
    int ignoredDeadTarget;
} Dm1V1ChampionMirrorClickResultPc34Compat;

void DM1_V1_ChampionMirror_InitClickStatePc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state);

int DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
    int x,
    int y,
    unsigned int mouseButtons);

int DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34Compat(
    int command,
    int partyChampionCount,
    unsigned int candidateChampionOrdinal,
    int *outChampionIndex,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult);

int DM1_V1_ChampionMirror_F0368SetLeaderPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int targetChampionIndex,
    Dm1V1ChampionMirrorClickResultPc34Compat *ioResult);

int DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int clickedChampionIndex,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult);

int DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult);

const char *DM1_V1_ChampionMirror_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
