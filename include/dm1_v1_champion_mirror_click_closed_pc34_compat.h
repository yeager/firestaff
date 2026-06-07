#ifndef DM1_V1_CHAMPION_MIRROR_CLICK_CLOSED_PC34_COMPAT_H
#define DM1_V1_CHAMPION_MIRROR_CLICK_CLOSED_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CLICK_CLOSED_MOUSE_LEFT_PC34_COMPAT 0x0002u

#define DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT 16

typedef struct Dm1V1MirrorClickClosedChampionPc34Compat {
    int currentHealth;
    int portraitOrdinal;
    unsigned int attributes;
} Dm1V1MirrorClickClosedChampionPc34Compat;

typedef struct Dm1V1MirrorClickClosedStatePc34Compat {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    int frontD1cMirrorChampionOrdinal;
    Dm1V1MirrorClickClosedChampionPc34Compat
        champions[DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorClickClosedStatePc34Compat;

typedef struct Dm1V1MirrorClickClosedResultPc34Compat {
    int clickedChampionIndex;
    int nestedCommand;
    int targetLeaderIndex;
    int previousLeaderIndex;
    int newLeaderIndex;
    int dispatchedStatusBoxClick;
    int scannedChampionNameRows;
    int oldLeaderDetached;
    int leaderChanged;
    int frontD1cPortraitIndex;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
} Dm1V1MirrorClickClosedResultPc34Compat;

void DM1_V1_MirrorClickClosed_InitPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state);

int DM1_V1_MirrorClickClosed_ProcessStatusBoxClickPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorClickClosedResultPc34Compat *outResult);

const char *DM1_V1_MirrorClickClosed_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
