#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"

#include <string.h>

typedef struct DM1_V1_ChampionMirrorNameZonePc34 {
    int command;
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_ChampionMirrorNameZonePc34;

static const DM1_V1_ChampionMirrorNameZonePc34 kNameZones[] = {
    { 16,   0,  42, 0, 6 },
    { 17,  69, 111, 0, 6 },
    { 18, 138, 180, 0, 6 },
    { 19, 207, 249, 0, 6 }
};

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex < DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT;
}

static void result_init(DM1_V1_ChampionMirrorClickResultPc34 *result,
                        const DM1_V1_ChampionMirrorClickStatePc34 *state)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->previousLeaderIndex = state ? state->leaderIndex :
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    result->newLeaderIndex = result->previousLeaderIndex;
    result->clickedChampionIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    result->nestedCommand = DM1_V1_COMMAND_NONE_PC34_COMPAT;
    result->targetLeaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
}

void DM1_V1_ChampionMirror_InitClickStatePc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    for (i = 0; i < DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT; ++i) {
        state->champions[i].currentHealth = 100;
    }
}

int DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
    int x,
    int y,
    unsigned int mouseButtons)
{
    unsigned int i;

    /* ReDMCSB: COMMAND.C:484-488 defines the PC-98/PC C159..C162
     * champion-name rows; COMMAND.C:1437-1449 F0358 matches inclusive
     * box coordinates only when the requested mouse button is present. */
    if ((mouseButtons & DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) == 0u) {
        return DM1_V1_COMMAND_NONE_PC34_COMPAT;
    }
    for (i = 0; i < sizeof(kNameZones) / sizeof(kNameZones[0]); ++i) {
        if (x >= kNameZones[i].left && x <= kNameZones[i].right &&
            y >= kNameZones[i].top && y <= kNameZones[i].bottom) {
            return kNameZones[i].command;
        }
    }
    return DM1_V1_COMMAND_NONE_PC34_COMPAT;
}

int DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34(
    int command,
    int partyChampionCount,
    unsigned int candidateChampionOrdinal,
    int *outChampionIndex,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult)
{
    int championIndex;

    if (outChampionIndex) {
        *outChampionIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    }
    if (command < DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT ||
        command > DM1_V1_COMMAND_CLICK_STATUS_BOX_3_PC34_COMPAT) {
        return 0;
    }
    championIndex = command - DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT;
    if (outChampionIndex) {
        *outChampionIndex = championIndex;
    }
    if (outResult) {
        outResult->clickedChampionIndex = championIndex;
    }

    /* ReDMCSB: COMMAND.C F0380 lines 2158-2162 reaches CLIKCHAM.C F0367
     * only when the clicked champion slot exists and G0299 is zero. */
    if (championIndex >= partyChampionCount) {
        if (outResult) {
            outResult->ignoredOutOfParty = 1;
        }
        return 0;
    }
    if (candidateChampionOrdinal != 0u) {
        if (outResult) {
            outResult->ignoredByCandidatePanel = 1;
        }
        return 0;
    }
    if (outResult) {
        outResult->dispatchesStatusBoxClick = 1;
    }
    return 1;
}

int DM1_V1_ChampionMirror_F0368SetLeaderPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int targetChampionIndex,
    DM1_V1_ChampionMirrorClickResultPc34 *ioResult)
{
    if (!state || !ioResult || !valid_champion_index(targetChampionIndex)) {
        return 0;
    }
    ioResult->attemptedSetLeader = 1;
    ioResult->targetLeaderIndex = targetChampionIndex;

    /* ReDMCSB: CLIKCHAM.C F0368 lines 51-53 returns for the current
     * leader or a dead non-none target. */
    if (targetChampionIndex == state->leaderIndex) {
        ioResult->ignoredSameLeader = 1;
        return 0;
    }
    if (state->champions[targetChampionIndex].currentHealth == 0) {
        ioResult->ignoredDeadTarget = 1;
        return 0;
    }

    /* ReDMCSB: CLIKCHAM.C F0368 lines 54-72 clears the old G0411 leader,
     * assigns the new leader, and skips only the redraw when the target is
     * the pending G0299 candidate.  The leader assignment itself still
     * belongs to F0368, which is why the C040 owner must block F0367 first. */
    state->leaderIndex = targetChampionIndex;
    ioResult->newLeaderIndex = targetChampionIndex;
    ioResult->leaderChanged = 1;
    return 1;
}

int DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int clickedChampionIndex,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult)
{
    int nestedCommand;
    int targetChampionIndex;

    result_init(outResult, state);
    if (!state || !outResult || !valid_champion_index(clickedChampionIndex)) {
        return 0;
    }
    outResult->clickedChampionIndex = clickedChampionIndex;

    /* ReDMCSB: CLIKCHAM.C F0367 lines 24-25 selects the clicked champion
     * directly when that status box belongs to the open inventory champion. */
    if ((unsigned int)(clickedChampionIndex + 1) ==
        state->inventoryChampionOrdinal) {
        return DM1_V1_ChampionMirror_F0368SetLeaderPc34(
            state, clickedChampionIndex, outResult);
    }

    nestedCommand = DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
        x, y, mouseButtons);
    outResult->nestedCommand = nestedCommand;

    /* ReDMCSB: CLIKCHAM.C F0367 lines 27-30 scans G0455 and maps
     * C016..C019 champion-name commands to F0368 target indices. */
    if (nestedCommand >= DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT &&
        nestedCommand <= DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT) {
        targetChampionIndex =
            nestedCommand - DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT;
        return DM1_V1_ChampionMirror_F0368SetLeaderPc34(
            state, targetChampionIndex, outResult);
    }
    return 0;
}

int DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
    DM1_V1_ChampionMirrorClickStatePc34 *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_ChampionMirrorClickResultPc34 *outResult)
{
    int championIndex;
    DM1_V1_ChampionMirrorClickResultPc34 outerResult;
    DM1_V1_ChampionMirrorClickResultPc34 innerResult;
    int changed;

    result_init(outResult, state);
    if (!state || !outResult) {
        return 0;
    }
    result_init(&outerResult, state);
    if (!DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34(
            command, state->partyChampionCount, state->candidateChampionOrdinal,
            &championIndex, &outerResult)) {
        *outResult = outerResult;
        return 0;
    }
    changed = DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34(
        state, championIndex, x, y, mouseButtons, &innerResult);
    innerResult.dispatchesStatusBoxClick = outerResult.dispatchesStatusBoxClick;
    innerResult.ignoredByCandidatePanel = outerResult.ignoredByCandidatePanel;
    innerResult.ignoredOutOfParty = outerResult.ignoredOutOfParty;
    innerResult.clickedChampionIndex = outerResult.clickedChampionIndex;
    *outResult = innerResult;
    return changed;
}

int DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
    int sensorType,
    int sensorData,
    int ornamentOrdinal,
    int thingCell,
    int visibleWallCell,
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 *outReceipt)
{
    if (!outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->championPortraitOrdinal =
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->championPortraitRenderIndex =
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;

    /* ReDMCSB DUNGEON.C F0172/F0174 lines 2573,2608-2612: C127
     * champion-portrait sensors count only on the wall face currently
     * seen by the party, then G0289 receives
     * M000_INDEX_TO_ORDINAL(M040_DATA(sensor)). COMPILE.H line 1038
     * defines M000_INDEX_TO_ORDINAL(value) as value + 1; Firestaff keeps
     * that 1-based source ordinal beside the 0-based C026 render index. */
    outReceipt->valid = 1;
    if (sensorType != 127 || thingCell != visibleWallCell) {
        return 1;
    }

    outReceipt->isFrontMirror = 1;
    outReceipt->championPortraitOrdinal = sensorData + 1;
    outReceipt->championPortraitRenderIndex = sensorData;
    if (ornamentOrdinal > 0) {
        outReceipt->wallOrnamentOrdinal = ornamentOrdinal;
    }
    return 1;
}

int DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
    const DM1_V1_ChampionMirrorFrontWallReceiptPc34 *frontWallReceipt,
    DM1_V1_ChampionMirrorRenderReceiptPc34 *outReceipt)
{
    DM1_FrontMirrorRenderPlanPc34 mirrorPlan;
    int renderIndex;

    if (!frontWallReceipt || !outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->valid = 1;
    outReceipt->sourceOrdinal =
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->renderIndex =
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->sourceAnchor =
        "ReDMCSB DUNVIEW.C:3913-3928 C026->G0109 champion portrait blit";

    if (!frontWallReceipt->valid ||
        !frontWallReceipt->isFrontMirror ||
        frontWallReceipt->championPortraitRenderIndex < 0) {
        outReceipt->suppressChampionPortrait = 1;
        outReceipt->suppressMaterializedItemPayload = 1;
        outReceipt->clearStaleMaterializedItemPayload = 1;
        return 1;
    }

    renderIndex = frontWallReceipt->championPortraitRenderIndex;
    /* C026 is exactly the ReDMCSB 8x3, 32x29 champion strip. Original HoC
     * sensors are valid, but a damaged/custom C127 record must not make M11
     * sample beyond that source atlas or replace it with host art. */
    if (renderIndex >= DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COUNT_PC34_COMPAT) {
        outReceipt->suppressChampionPortrait = 1;
        outReceipt->suppressMaterializedItemPayload = 1;
        outReceipt->clearStaleChampionPortraitOrdinal = 1;
        outReceipt->clearStaleMaterializedItemPayload = 1;
        return 1;
    }
    outReceipt->drawChampionPortrait = 1;
    outReceipt->drawMirrorBacking = 1;
    outReceipt->sourceOrdinal = frontWallReceipt->championPortraitOrdinal;
    outReceipt->renderIndex = renderIndex;
    outReceipt->graphicIndex =
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT;
    outReceipt->sourceX =
        (renderIndex %
         DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COLS_PC34_COMPAT) *
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_WIDTH_PC34_COMPAT;
    outReceipt->sourceY =
        (renderIndex /
         DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COLS_PC34_COMPAT) *
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_HEIGHT_PC34_COMPAT;
    outReceipt->width = DM1_V1_CHAMPION_MIRROR_PORTRAIT_WIDTH_PC34_COMPAT;
    outReceipt->height = DM1_V1_CHAMPION_MIRROR_PORTRAIT_HEIGHT_PC34_COMPAT;
    outReceipt->dstX = DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_X_PC34_COMPAT;
    outReceipt->dstY = DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_Y_PC34_COMPAT;
    outReceipt->frameLeft =
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_LEFT_PC34_COMPAT;
    outReceipt->frameRight =
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_RIGHT_PC34_COMPAT;
    outReceipt->frameTop =
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_TOP_PC34_COMPAT;
    outReceipt->frameBottom =
        DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_BOTTOM_PC34_COMPAT;
    outReceipt->transparentColor =
        DM1_V1_CHAMPION_MIRROR_TRANSPARENT_COLOR_PC34_COMPAT;
    if (dm1_v1_front_mirror_render_plan_pc34(renderIndex, &mirrorPlan)) {
        int i;
        outReceipt->backingGraphicIndex = mirrorPlan.ornament.graphicIndex;
        outReceipt->backingSourceX = mirrorPlan.ornament.srcX;
        outReceipt->backingSourceY = mirrorPlan.ornament.srcY;
        outReceipt->backingSourceWidth = mirrorPlan.backingSourceWidth;
        outReceipt->backingSourceHeight = mirrorPlan.backingSourceHeight;
        outReceipt->backingDstX = mirrorPlan.backingDstX;
        outReceipt->backingDstY = mirrorPlan.backingDstY;
        outReceipt->backingWidth = mirrorPlan.backingWidth;
        outReceipt->backingHeight = mirrorPlan.backingHeight;
        outReceipt->backingTransparentColor =
            mirrorPlan.ornament.transparentColor;
        outReceipt->backingFlipHorizontal =
            mirrorPlan.ornament.flipHorizontal;
        outReceipt->backingPaletteMapValid =
            mirrorPlan.ornament.paletteMapValid;
        for (i = 0; i < 16; ++i) {
            outReceipt->backingPaletteMap[i] =
                mirrorPlan.ornament.paletteMap[i];
        }
    } else {
        outReceipt->drawMirrorBacking = 0;
    }

    /* ReDMCSB: DUNVIEW.C lines 3913-3928 only blit C026 champion
     * portraits when the wall is D1C front and G0289 decrements to a valid
     * source ordinal; lines 3922-3928 draw C346 backing before C026. Keep
     * both draw/material decisions in this DM1 receipt so host render code
     * does not rebuild a second mirror plan or reuse stale payload state. */
    return 1;
}

int DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
    int wallSquareVisible,
    const DM1_V1_ChampionMirrorFrontWallReceiptPc34 *frontWallReceipt,
    DM1_V1_ChampionMirrorRenderReceiptPc34 *outReceipt)
{
    if (!DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
            frontWallReceipt, outReceipt)) {
        return 0;
    }

    outReceipt->consumedWallSquareReceipt = wallSquareVisible ? 1 : 0;
    if (wallSquareVisible && !outReceipt->drawChampionPortrait) {
        /* ReDMCSB: DUNGEON.C line 2558 resets G0289 when a wall square
         * contributes to the dungeon view; DUNVIEW.C lines 3913-3928 then
         * consumes only the current D1C front portrait ordinal. Keep that
         * stale-clear decision in the DM1 receipt so HoC render hosts do not
         * carry champion mirror payloads into normal item/projectile layers. */
        outReceipt->clearStaleChampionPortraitOrdinal = 1;
        outReceipt->clearStaleMaterializedItemPayload = 1;
        outReceipt->suppressChampionPortrait = 1;
        outReceipt->suppressMaterializedItemPayload = 1;
    }
    return 1;
}

int DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34 *renderReceipt,
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 *outReceipt)
{
    if (!renderReceipt || !outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->sourceOrdinal = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->renderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->sourceAnchor =
        "ReDMCSB DUNVIEW.C:3913-3928 C026 wall overlay; "
        "DUNVIEW.C:4547-4581 F0115 thing pass";
    if (!renderReceipt->valid) {
        return 1;
    }

    outReceipt->valid = 1;
    outReceipt->consumedRenderReceipt = 1;
    outReceipt->sourceOrdinal = renderReceipt->sourceOrdinal;
    outReceipt->renderIndex = renderReceipt->renderIndex;
    outReceipt->graphicIndex = renderReceipt->graphicIndex;
    outReceipt->allowIndependentFloorObjects = 1;
    outReceipt->requireRuntimeProjectileReceipt = 1;

    /* ReDMCSB DUNVIEW.C F0107 lines 3913-3928 draws the C026 champion
     * portrait through the D1C front wall ornament path.  F0115 lines
     * 4547-4581 owns floor objects/projectiles separately, so the mirror
     * payload must not be reclassified as a floor item, projectile, or spell
     * effect while real floor objects stay available to the thing pass. */
    outReceipt->drawChampionPortraitAsWallOverlay =
        renderReceipt->drawChampionPortrait ? 1 : 0;
    outReceipt->suppressMirrorAsFloorItem = 1;
    outReceipt->suppressMirrorAsProjectile = 1;
    outReceipt->suppressMirrorAsSpellEffect = 1;
    outReceipt->suppressMaterializedItemPayload =
        renderReceipt->suppressMaterializedItemPayload ||
        renderReceipt->drawChampionPortrait;
    outReceipt->thingLayerSafe =
        outReceipt->suppressMirrorAsFloorItem &&
        outReceipt->suppressMirrorAsProjectile &&
        outReceipt->suppressMirrorAsSpellEffect &&
        outReceipt->allowIndependentFloorObjects &&
        outReceipt->requireRuntimeProjectileReceipt;
    return 1;
}

int DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
    const DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 *boundaryReceipt,
    const DM1V1D1LD1RF0115RuntimeThingReceiptPc34 *runtimeThingReceipt,
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 *outReceipt)
{
    if (!boundaryReceipt || !runtimeThingReceipt || !outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->sourceOrdinal = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->renderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->zone = -1;
    outReceipt->row = -1;
    outReceipt->viewCell = -1;
    outReceipt->sourceAnchor =
        "ReDMCSB DUNVIEW.C:3913-3928 C026 wall overlay; "
        "DUNVIEW.C:4547-4581/5075/5668-5683 F0115 runtime thing layers";
    if (!boundaryReceipt->valid || !runtimeThingReceipt->valid) {
        return 1;
    }

    outReceipt->valid = 1;
    outReceipt->consumedBoundaryReceipt = 1;
    outReceipt->consumedRuntimeThingReceipt = 1;
    outReceipt->sourceOrdinal = boundaryReceipt->sourceOrdinal;
    outReceipt->renderIndex = boundaryReceipt->renderIndex;
    outReceipt->zone = runtimeThingReceipt->zone;
    outReceipt->row = runtimeThingReceipt->row;
    outReceipt->viewCell = runtimeThingReceipt->view_cell;

    /* ReDMCSB: DUNVIEW.C lines 3913-3928 render C026 champion portraits
     * only through the D1C front wall overlay path.  The later F0115 pass
     * (4547-4581, floor item zone near 5075, projectile path 5668-5683)
     * consumes runtime thing/projectile receipts, so this DM1 receipt lets
     * real floor objects/projectiles render without treating the mirror
     * payload as an item, projectile, or spell effect. */
    outReceipt->wallOverlayOnly =
        boundaryReceipt->drawChampionPortraitAsWallOverlay ? 1 : 0;
    outReceipt->drawChampionPortraitAsWallOverlay =
        boundaryReceipt->drawChampionPortraitAsWallOverlay ? 1 : 0;
    outReceipt->suppressMirrorAsFloorItem =
        boundaryReceipt->suppressMirrorAsFloorItem ? 1 : 0;
    outReceipt->suppressMirrorAsProjectile =
        boundaryReceipt->suppressMirrorAsProjectile ? 1 : 0;
    outReceipt->suppressMirrorAsSpellEffect =
        boundaryReceipt->suppressMirrorAsSpellEffect ? 1 : 0;
    outReceipt->suppressMaterializedItemPayload =
        boundaryReceipt->suppressMaterializedItemPayload ? 1 : 0;
    outReceipt->floorObjectLayerAllowed =
        boundaryReceipt->allowIndependentFloorObjects ? 1 : 0;
    outReceipt->runtimeProjectileReceiptRequired =
        boundaryReceipt->requireRuntimeProjectileReceipt ? 1 : 0;
    outReceipt->drawFloorObject =
        outReceipt->floorObjectLayerAllowed &&
        runtimeThingReceipt->input_valid &&
        runtimeThingReceipt->draw_item &&
        !runtimeThingReceipt->suppress_item;
    outReceipt->drawRuntimeProjectile =
        outReceipt->runtimeProjectileReceiptRequired &&
        runtimeThingReceipt->input_valid &&
        runtimeThingReceipt->consumes_runtime_projectile_list &&
        runtimeThingReceipt->draw_projectile &&
        !runtimeThingReceipt->suppress_projectile;
    outReceipt->thingLayerSafe =
        boundaryReceipt->thingLayerSafe &&
        outReceipt->suppressMirrorAsFloorItem &&
        outReceipt->suppressMirrorAsProjectile &&
        outReceipt->suppressMirrorAsSpellEffect &&
        (outReceipt->drawFloorObject ||
         outReceipt->drawRuntimeProjectile ||
         runtimeThingReceipt->suppress_non_thing_payload);
    return 1;
}

int DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34 *renderReceipt,
    int candidatePanelActive,
    int backingAssetAvailable,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 *outReceipt)
{
    int i;

    if (!renderReceipt || !outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->sourceOrdinal = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->renderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    outReceipt->sourceAnchor =
        "ReDMCSB DUNGEON.C:2608-2612 G0289; "
        "DUNVIEW.C:3913-3928 C346/C026 front mirror draw";
    if (!renderReceipt->valid || !renderReceipt->drawChampionPortrait) {
        return 1;
    }

    outReceipt->valid = 1;
    outReceipt->consumedRenderReceipt = 1;
    outReceipt->candidatePanelOwnsCell = candidatePanelActive ? 1 : 0;
    outReceipt->drawChampionPortrait = 1;
    outReceipt->sourceOrdinal = renderReceipt->sourceOrdinal;
    outReceipt->renderIndex = renderReceipt->renderIndex;
    outReceipt->portraitGraphicIndex = renderReceipt->graphicIndex;
    outReceipt->portraitSourceX = renderReceipt->sourceX;
    outReceipt->portraitSourceY = renderReceipt->sourceY;
    outReceipt->portraitWidth = renderReceipt->width;
    outReceipt->portraitHeight = renderReceipt->height;
    outReceipt->portraitDstX = renderReceipt->dstX;
    outReceipt->portraitDstY = renderReceipt->dstY;
    outReceipt->portraitTransparentColor = renderReceipt->transparentColor;
    outReceipt->backingGraphicIndex = renderReceipt->backingGraphicIndex;
    outReceipt->backingSourceX = renderReceipt->backingSourceX;
    outReceipt->backingSourceY = renderReceipt->backingSourceY;
    outReceipt->backingSourceWidth = renderReceipt->backingSourceWidth;
    outReceipt->backingSourceHeight = renderReceipt->backingSourceHeight;
    outReceipt->backingDstX = renderReceipt->backingDstX;
    outReceipt->backingDstY = renderReceipt->backingDstY;
    outReceipt->backingWidth = renderReceipt->backingWidth;
    outReceipt->backingHeight = renderReceipt->backingHeight;
    outReceipt->backingTransparentColor =
        renderReceipt->backingTransparentColor;
    outReceipt->backingFlipHorizontal = renderReceipt->backingFlipHorizontal;
    outReceipt->backingPaletteMapValid =
        renderReceipt->backingPaletteMapValid;
    for (i = 0; i < 16; ++i) {
        outReceipt->backingPaletteMap[i] = renderReceipt->backingPaletteMap[i];
    }

    /* ReDMCSB draws C346 before C026 in the front wall route.  When the C040
     * champion panel owns the cell, the wall ornament asset is suppressed; the
     * portrait remains the live mirror candidate.  Otherwise this DM1 receipt
     * owns whether the host should draw the extracted C346 bitmap or the
     * invariant backing rectangle. */
    if (outReceipt->candidatePanelOwnsCell) {
        outReceipt->suppressWallOrnamentAsset = 1;
        outReceipt->suppressHostFallbackVisuals = 1;
        return 1;
    }

    outReceipt->drawMirrorBackingAsset =
        renderReceipt->drawMirrorBacking && backingAssetAvailable ? 1 : 0;
    outReceipt->drawMirrorBackingFallbackRect =
        renderReceipt->drawMirrorBacking && !backingAssetAvailable ? 1 : 0;
    outReceipt->drawInvariantBackingRect =
        renderReceipt->drawMirrorBacking ? 1 : 0;
    outReceipt->suppressHostFallbackVisuals = 1;
    return 1;
}

int DM1_V1_ChampionMirror_BuildSourceOwnedHostDrawReceiptPc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34 *renderReceipt,
    int candidatePanelActive,
    int backingAssetAvailable,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 *outReceipt)
{
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 receipt;

    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
            renderReceipt, candidatePanelActive, backingAssetAvailable,
            &receipt)) {
        *outReceipt = receipt;
        return 0;
    }

    /* DUNVIEW.C:3913-3928 owns the C346-before-C026 pair.  The old generic
     * receipt can describe a rectangle for compatibility probes, but neither
     * the live HoC renderer nor the entrance hand-off may execute it. */
    if (!receipt.valid ||
        (!candidatePanelActive &&
         (!backingAssetAvailable || !receipt.drawMirrorBackingAsset))) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.sourceAnchor =
            "ReDMCSB DUNVIEW.C:3913-3928 C346/C026 source-owned no-draw";
        receipt.suppressHostFallbackVisuals = 1;
        *outReceipt = receipt;
        return 1;
    }

    receipt.drawMirrorBackingFallbackRect = 0;
    receipt.drawInvariantBackingRect = 0;
    receipt.sourceAssetsBound = 1;
    receipt.sourceOwnedExecution = 1;
    receipt.suppressHostFallbackVisuals = 1;
    *outReceipt = receipt;
    return 1;
}

int DM1_V1_ChampionMirror_ValidateHostMaterialReceiptPc34(
    const DM1_V1_ChampionMirrorHostDrawReceiptPc34 *receipt,
    int backingAssetWidth,
    int backingAssetHeight,
    int portraitAssetWidth,
    int portraitAssetHeight)
{
    DM1_FrontMirrorRenderPlanPc34 plan;

    if (!receipt || !receipt->valid || !receipt->sourceAssetsBound ||
        !receipt->sourceOwnedExecution || !receipt->drawChampionPortrait ||
        !receipt->suppressHostFallbackVisuals ||
        !dm1_v1_front_mirror_render_plan_pc34(receipt->renderIndex, &plan) ||
        receipt->portraitGraphicIndex != plan.portraitGraphicIndex ||
        receipt->portraitSourceX != plan.portraitSrcX ||
        receipt->portraitSourceY != plan.portraitSrcY ||
        receipt->portraitWidth != plan.portraitWidth ||
        receipt->portraitHeight != plan.portraitHeight ||
        receipt->portraitDstX != plan.portraitDstX ||
        receipt->portraitDstY != plan.portraitDstY ||
        receipt->portraitTransparentColor != plan.portraitTransparentColor ||
        !dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
            portraitAssetWidth, portraitAssetHeight) ||
        receipt->portraitSourceX + receipt->portraitWidth > portraitAssetWidth ||
        receipt->portraitSourceY + receipt->portraitHeight > portraitAssetHeight) {
        return 0;
    }

    if (receipt->candidatePanelOwnsCell) {
        return !receipt->drawMirrorBackingAsset;
    }
    return receipt->drawMirrorBackingAsset &&
        receipt->backingGraphicIndex == plan.ornament.graphicIndex &&
        receipt->backingSourceX == plan.ornament.srcX &&
        receipt->backingSourceY == plan.ornament.srcY &&
        receipt->backingSourceWidth == plan.backingSourceWidth &&
        receipt->backingSourceHeight == plan.backingSourceHeight &&
        receipt->backingDstX == plan.backingDstX &&
        receipt->backingDstY == plan.backingDstY &&
        receipt->backingWidth == plan.backingWidth &&
        receipt->backingHeight == plan.backingHeight &&
        receipt->backingTransparentColor == plan.ornament.transparentColor &&
        receipt->backingFlipHorizontal == plan.ornament.flipHorizontal &&
        receipt->backingPaletteMapValid == plan.ornament.paletteMapValid &&
        memcmp(receipt->backingPaletteMap, plan.ornament.paletteMap,
               sizeof(plan.ornament.paletteMap)) == 0 &&
        backingAssetWidth >= receipt->backingSourceX + receipt->backingSourceWidth &&
        backingAssetHeight >= receipt->backingSourceY + receipt->backingSourceHeight;
}

static int dm1_v1_champion_mirror_is_f0107_wall_projection_pc34(
    int relForward,
    int relSide,
    int viewWallIndex)
{
    /* DUNVIEW.C walks one 15-entry Graphic558 coordinate set for every
     * F0107 wall-ornament projection.  C127 changes the ornament source to
     * C346, not the perspective slot itself. */
    static const struct {
        int relForward;
        int relSide;
        int viewWallIndex;
    } projections[] = {
        {3, -2, 0}, {3,  2, 1}, {3, -1, 2}, {3, 0, 3},
        {3,  1, 4}, {2, -1, 5}, {2,  1, 6}, {2, -1, 7},
        {2,  0, 8}, {2,  1, 9}, {1, -1,10}, {1, 1,11}
    };
    size_t index;

    for (index = 0; index < sizeof(projections) / sizeof(projections[0]);
         ++index) {
        if (projections[index].relForward == relForward &&
            projections[index].relSide == relSide &&
            projections[index].viewWallIndex == viewWallIndex) {
            return 1;
        }
    }
    return 0;
}

int DM1_V1_ChampionMirror_BuildViewportProjectionReceiptPc34(
    int relForward,
    int relSide,
    int viewWallIndex,
    int championPortraitOrdinal,
    int globalOrnamentIndex,
    DM1_V1_ChampionMirrorViewportProjectionReceiptPc34 *outReceipt)
{
    DM1_V1_ChampionMirrorViewportProjectionReceiptPc34 receipt;

    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.relForward = relForward;
    receipt.relSide = relSide;
    receipt.viewWallIndex = viewWallIndex;
    receipt.globalOrnamentIndex = globalOrnamentIndex;
    receipt.sourceAnchor =
        "ReDMCSB DUNGEON.C:2608-2612 C127/G0289 wall fact; "
        "DUNVIEW.C F0107:3502-3938 wall-ornament projection; "
        "DUNVIEW.C:3913-3928 D1C C346/C026 portrait overlay";

    if (championPortraitOrdinal < 0) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.consumedC127WallFact = 1;
    receipt.suppressHostFallbackVisuals = 1;
    receipt.suppressChampionPortrait = 1;

    if (relForward == 1 && relSide == 0 && viewWallIndex == 12) {
        receipt.drawChampionPortrait = 1;
        receipt.suppressChampionPortrait = 0;
        receipt.suppressGenericWallOrnament = 1;
        *outReceipt = receipt;
        return 1;
    }

    /* C026 is a D1C-only portrait overlay, but C346 remains a normal wall
     * ornament in every other real F0107 perspective slot.  Suppressing it
     * at D2/D3 made a Hall mirror frame disappear until the player stood
     * immediately in front of it. */
    if (dm1_v1_champion_mirror_is_f0107_wall_projection_pc34(
            relForward, relSide, viewWallIndex) &&
        globalOrnamentIndex ==
            DM1_V1_CHAMPION_MIRROR_BACKING_GLOBAL_ORNAMENT_PC34_COMPAT) {
        receipt.drawWallOrnamentBacking = 1;
    } else {
        receipt.suppressGenericWallOrnament = 1;
    }

    *outReceipt = receipt;
    return 1;
}

int DM1_V1_ChampionMirror_BuildRuntimeRenderDecisionPc34(
    const DM1_V1_ChampionMirrorRuntimeRenderInputPc34 *input,
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *outDecision)
{
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 decision;

    if (!input || !input->runtimeThingReceipt || !outDecision) {
        return 0;
    }
    memset(&decision, 0, sizeof(decision));
    if (!DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            input->sensorType, input->sensorData, input->ornamentOrdinal,
            input->thingCell, input->visibleWallCell, &decision.frontWall) ||
        !DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
            input->wallSquareVisible, &decision.frontWall, &decision.render) ||
        !DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
            &decision.render, &decision.thingBoundary) ||
        !DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
            &decision.thingBoundary, input->runtimeThingReceipt,
            &decision.thingConsumer) ||
        !DM1_V1_ChampionMirror_BuildSourceOwnedHostDrawReceiptPc34(
            &decision.render, input->candidatePanelActive,
            input->backingAssetAvailable, &decision.hostDraw)) {
        return 0;
    }

    /* ReDMCSB DUNGEON.C:2608-2612 publishes C127 only for the visible
     * wall face; DUNVIEW.C:3913-3928 consumes it before F0115's separate
     * object/projectile pass.  Keep that ordering as one DM1-owned result. */
    decision.valid = 1;
    decision.consumedF0172Sensor = 1;
    decision.consumedF0115ThingReceipt = decision.thingConsumer.valid;
    decision.drawFrontWallOverlay =
        decision.hostDraw.sourceOwnedExecution &&
        decision.render.drawMirrorBacking;
    decision.drawChampionPortraitAsWallOverlay =
        decision.thingConsumer.drawChampionPortraitAsWallOverlay;
    decision.drawFloorObject = decision.thingConsumer.drawFloorObject;
    decision.drawRuntimeProjectile = decision.thingConsumer.drawRuntimeProjectile;
    /* F0115's later explosion restart is independent of the mirror.  M11
     * must receive an explicit receipt before it can draw that material. */
    decision.drawRuntimeSpellEffect =
        decision.thingConsumer.runtimeProjectileReceiptRequired ? 0 : 1;
    decision.suppressMaterializedItemPayload =
        decision.thingConsumer.suppressMaterializedItemPayload;
    decision.suppressMirrorAsFloorItem =
        decision.thingConsumer.suppressMirrorAsFloorItem;
    decision.suppressMirrorAsProjectile =
        decision.thingConsumer.suppressMirrorAsProjectile;
    decision.suppressMirrorAsSpellEffect =
        decision.thingConsumer.suppressMirrorAsSpellEffect;
    decision.suppressHostFallbackVisuals =
        decision.hostDraw.suppressHostFallbackVisuals;
    *outDecision = decision;
    return 1;
}

const char *DM1_V1_ChampionMirror_SourceEvidencePc34(void)
{
    return "ReDMCSB COMMAND.C:484-488 G0455 maps C159..C162 champion-name "
           "zones to C016..C019; COMMAND.C:1379-1449 F0358 matches inclusive "
           "mouse boxes; COMMAND.C:2158-2162 F0380 dispatches champion status "
           "boxes only when G0299 is clear; CLIKCHAM.C:24-35 F0367 maps "
           "status/name clicks to F0368; CLIKCHAM.C:51-72 F0368 changes "
           "G0411 leader and skips redraw for the G0299 candidate; "
           "DUNGEON.C:2573,2608-2612 F0172/F0174 and COMPILE.H:1038 map "
           "C127 front-wall sensor data to the 1-based G0289 champion "
           "portrait ordinal; DUNVIEW.C:3913-3928 blits C026 champion "
           "portraits through G0109 {96,127,35,63}; DUNVIEW.C F0107:3502-3938 "
           "still projects the source wall ornament for side/depth C127 "
           "mirror walls while suppressing C026 outside D1C; DUNGEON.C:2558 resets "
           "G0289 when a wall square is present, preventing stale mirror "
           "payloads from leaking into later dungeon-view layers; "
           "DUNVIEW.C:4547-4581,5075,5668-5683 keep F0115 floor items and "
           "runtime projectiles separate from the C026 wall-overlay path.";
}
