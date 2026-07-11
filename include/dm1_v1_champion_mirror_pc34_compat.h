#ifndef DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H
#define DM1_V1_CHAMPION_MIRROR_PC34_COMPAT_H

/*
 * pass796 - DM1 V1 champion mirror contract test
 * (COMMAND.C:484-488 PC-98/PC C159..C162 champion-name rows;
 * COMMAND.C:1437-1449 F0358 inclusive match; F0380:2158-2162
 * status-box click dispatch). Source-locked against COMMAND.C
 * F0358:1437-1449 + F0380:2158-2162 + DEFS.H C159..C162.
 */

#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

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
    int drawMirrorBacking;
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
    int backingGraphicIndex;
    int backingSourceX;
    int backingSourceY;
    int backingDstX;
    int backingDstY;
    int backingWidth;
    int backingHeight;
    int backingTransparentColor;
    int backingFlipHorizontal;
    int backingPaletteMapValid;
    unsigned char backingPaletteMap[16];
    const char *sourceAnchor;
} DM1_V1_ChampionMirrorRenderReceiptPc34;

typedef struct DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 {
    int valid;
    int consumedRenderReceipt;
    int drawChampionPortraitAsWallOverlay;
    int suppressMirrorAsFloorItem;
    int suppressMirrorAsProjectile;
    int suppressMirrorAsSpellEffect;
    int suppressMaterializedItemPayload;
    int allowIndependentFloorObjects;
    int requireRuntimeProjectileReceipt;
    int thingLayerSafe;
    int sourceOrdinal;
    int renderIndex;
    int graphicIndex;
    const char *sourceAnchor;
} DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34;

typedef struct DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 {
    int valid;
    int consumedBoundaryReceipt;
    int consumedRuntimeThingReceipt;
    int wallOverlayOnly;
    int drawChampionPortraitAsWallOverlay;
    int drawFloorObject;
    int drawRuntimeProjectile;
    int suppressMirrorAsFloorItem;
    int suppressMirrorAsProjectile;
    int suppressMirrorAsSpellEffect;
    int suppressMaterializedItemPayload;
    int floorObjectLayerAllowed;
    int runtimeProjectileReceiptRequired;
    int thingLayerSafe;
    int zone;
    int row;
    int viewCell;
    int sourceOrdinal;
    int renderIndex;
    const char *sourceAnchor;
} DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34;

typedef struct DM1_V1_ChampionMirrorHostDrawReceiptPc34 {
    int valid;
    int consumedRenderReceipt;
    int candidatePanelOwnsCell;
    int drawChampionPortrait;
    int drawMirrorBackingAsset;
    int drawMirrorBackingFallbackRect;
    int drawInvariantBackingRect;
    int suppressWallOrnamentAsset;
    int suppressHostFallbackVisuals;
    int sourceOrdinal;
    int renderIndex;
    int portraitGraphicIndex;
    int portraitSourceX;
    int portraitSourceY;
    int portraitWidth;
    int portraitHeight;
    int portraitDstX;
    int portraitDstY;
    int portraitTransparentColor;
    int backingGraphicIndex;
    int backingSourceX;
    int backingSourceY;
    int backingDstX;
    int backingDstY;
    int backingWidth;
    int backingHeight;
    int backingTransparentColor;
    int backingFlipHorizontal;
    int backingPaletteMapValid;
    unsigned char backingPaletteMap[16];
    const char *sourceAnchor;
} DM1_V1_ChampionMirrorHostDrawReceiptPc34;

/*
 * Runtime hand-off for the HoC D1C mirror path.  M11 supplies the already
 * decoded F0172/F0115 facts; DM1 owns the resulting wall, item, projectile
 * and materialized-payload decisions.
 */
typedef struct DM1_V1_ChampionMirrorRuntimeRenderInputPc34 {
    int wallSquareVisible;
    int sensorType;
    int sensorData;
    int ornamentOrdinal;
    int thingCell;
    int visibleWallCell;
    int candidatePanelActive;
    int backingAssetAvailable;
    const DM1V1D1LD1RF0115RuntimeThingReceiptPc34 *runtimeThingReceipt;
} DM1_V1_ChampionMirrorRuntimeRenderInputPc34;

typedef struct DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 {
    int valid;
    int consumedF0172Sensor;
    int consumedF0115ThingReceipt;
    /* The only M11 execution switches for a visible HoC D1C cell.  They
     * preserve DUNVIEW's wall-overlay -> object -> projectile separation. */
    int drawFrontWallOverlay;
    int drawChampionPortraitAsWallOverlay;
    int drawFloorObject;
    int drawRuntimeProjectile;
    int drawRuntimeSpellEffect;
    int suppressHostFallbackVisuals;
    int suppressMaterializedItemPayload;
    int suppressMirrorAsFloorItem;
    int suppressMirrorAsProjectile;
    int suppressMirrorAsSpellEffect;
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 frontWall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 thingBoundary;
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 thingConsumer;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 hostDraw;
} DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34;

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

int DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34 *renderReceipt,
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 *outReceipt);

int DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
    const DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 *boundaryReceipt,
    const DM1V1D1LD1RF0115RuntimeThingReceiptPc34 *runtimeThingReceipt,
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 *outReceipt);

int DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34 *renderReceipt,
    int candidatePanelActive,
    int backingAssetAvailable,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 *outReceipt);

int DM1_V1_ChampionMirror_BuildRuntimeRenderDecisionPc34(
    const DM1_V1_ChampionMirrorRuntimeRenderInputPc34 *input,
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *outDecision);

const char *DM1_V1_ChampionMirror_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
