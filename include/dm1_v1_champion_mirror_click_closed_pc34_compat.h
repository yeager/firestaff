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

typedef struct DM1_V1_MirrorClickClosedChampionPc34 {
    int currentHealth;
    int portraitOrdinal;
    unsigned int attributes;
} DM1_V1_MirrorClickClosedChampionPc34;

typedef struct DM1_V1_MirrorClickClosedStatePc34 {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    int frontD1cMirrorChampionOrdinal;
    DM1_V1_MirrorClickClosedChampionPc34
        champions[DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT];
} DM1_V1_MirrorClickClosedStatePc34;

typedef struct DM1_V1_MirrorClickClosedResultPc34 {
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
} DM1_V1_MirrorClickClosedResultPc34;

void DM1_V1_MirrorClickClosed_InitPc34(
    DM1_V1_MirrorClickClosedStatePc34 *state);

int DM1_V1_MirrorClickClosed_ProcessStatusBoxClickPc34(
    DM1_V1_MirrorClickClosedStatePc34 *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_MirrorClickClosedResultPc34 *outResult);

const char *DM1_V1_MirrorClickClosed_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
