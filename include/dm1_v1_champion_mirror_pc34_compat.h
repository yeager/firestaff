#ifndef DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H
#define DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H

/*
 * pass796 - DM1 V1 champion mirror contract test
 * (COMMAND.C:484-488 PC-98/PC C159..C162 champion-name rows;
 * COMMAND.C:1437-1449 F0358 inclusive match; F0380:2158-2162
 * status-box click dispatch). Source-locked against COMMAND.C
 * F0358:1437-1449 + F0380:2158-2162 + DEFS.H C159..C162.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT 4
#define DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT (-1)

#define DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT 0x0002u
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT 26
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_WIDTH_PC34_COMPAT 32
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_HEIGHT_PC34_COMPAT 29
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COLS_PC34_COMPAT 8
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_X_PC34_COMPAT 96
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_Y_PC34_COMPAT 35
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_LEFT_PC34_COMPAT 96
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_RIGHT_PC34_COMPAT 127
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_TOP_PC34_COMPAT 35
#define DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_BOTTOM_PC34_COMPAT 63
#define DM1_V1_CHAMPION_MIRROR_TRANSPARENT_COLOR_PC34_COMPAT 1

#define DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_COMMAND_CLICK_STATUS_BOX_3_PC34_COMPAT 15
#define DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT 16
#define DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT 19
#define DM1_V1_COMMAND_NONE_PC34_COMPAT 0

typedef struct DM1_V1_ChampionMirrorChampionPc34 {
    int currentHealth;
} DM1_V1_ChampionMirrorChampionPc34;

typedef struct DM1_V1_ChampionMirrorClickStatePc34 {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    DM1_V1_ChampionMirrorChampionPc34
        champions[DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT];
} DM1_V1_ChampionMirrorClickStatePc34;

typedef struct DM1_V1_ChampionMirrorClickResultPc34 {
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
} DM1_V1_ChampionMirrorClickResultPc34;

typedef struct DM1_V1_ChampionMirrorFrontWallReceiptPc34 {
    int valid;
    int isFrontMirror;
    int championPortraitOrdinal;       /* ReDMCSB G0289, 1-based source ordinal */
    int championPortraitRenderIndex;   /* C026 atlas index, 0-based */
    int wallOrnamentOrdinal;
} DM1_V1_ChampionMirrorFrontWallReceiptPc34;

typedef struct DM1_V1_ChampionMirrorRenderReceiptPc34 {
    int valid;
    int drawChampionPortrait;
    int suppressChampionPortrait;
    int suppressMaterializedItemPayload;
    int clearStaleChampionPortraitOrdinal;
    int clearStaleMaterializedItemPayload;
    int consumedWallSquareReceipt;
    int sourceOrdinal;
    int renderIndex;
    int graphicIndex;
    int sourceX;
    int sourceY;
    int width;
    int height;
    int dstX;
    int dstY;
    int frameLeft;
    int frameRight;
    int frameTop;
    int frameBottom;
    int transparentColor;
    const char *sourceAnchor;
} DM1_V1_ChampionMirrorRenderReceiptPc34;

void DM1_V1_ChampionMirror_InitClickStatePc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state);

int DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
    int x,
    int y,
    unsigned int mouseButtons);

int DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34(
    int command,
    int partyChampionCount,
    unsigned int candidateChampionOrdinal,
    int *outChampionIndex,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult);

int DM1_V1_ChampionMirror_F0368SetLeaderPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int targetChampionIndex,
    DM1_V1_ChampionMirrorClickResultPc34 *ioResult);

int DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int clickedChampionIndex,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult);

int DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult);

int DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
    int sensorType,
    int sensorData,
    int ornamentOrdinal,
    int thingCell,
    int visibleWallCell,
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 *outReceipt);

int DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
    const DM1_V1_ChampionMirrorFrontWallReceiptPc34 *frontWallReceipt,
    DM1_V1_ChampionMirrorRenderReceiptPc34 *outReceipt);

int DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
    int wallSquareVisible,
    const DM1_V1_ChampionMirrorFrontWallReceiptPc34 *frontWallReceipt,
    DM1_V1_ChampionMirrorRenderReceiptPc34 *outReceipt);

const char *DM1_V1_ChampionMirror_SourceEvidencePc34(void);

typedef DM1_V1_ChampionMirrorChampionPc34
    Dm1V1ChampionMirrorChampionPc34Compat;
typedef DM1_V1_ChampionMirrorClickStatePc34
    Dm1V1ChampionMirrorClickStatePc34Compat;
typedef DM1_V1_ChampionMirrorClickResultPc34
    Dm1V1ChampionMirrorClickResultPc34Compat;
typedef DM1_V1_ChampionMirrorFrontWallReceiptPc34
    Dm1V1ChampionMirrorFrontWallReceiptPc34Compat;
typedef DM1_V1_ChampionMirrorRenderReceiptPc34
    Dm1V1ChampionMirrorRenderReceiptPc34Compat;

#define DM1_V1_ChampionMirror_InitClickStatePc34Compat \
    DM1_V1_ChampionMirror_InitClickStatePc34
#define DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat \
    DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34
#define DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34Compat \
    DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34
#define DM1_V1_ChampionMirror_F0368SetLeaderPc34Compat \
    DM1_V1_ChampionMirror_F0368SetLeaderPc34
#define DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34Compat \
    DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34
#define DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat \
    DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34
#define DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34Compat \
    DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34
#define DM1_V1_ChampionMirror_BuildRenderReceiptPc34Compat \
    DM1_V1_ChampionMirror_BuildRenderReceiptPc34
#define DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34Compat \
    DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34
#define DM1_V1_ChampionMirror_SourceEvidencePc34Compat \
    DM1_V1_ChampionMirror_SourceEvidencePc34

#ifdef __cplusplus
}
#endif

#endif
