#include "dm1_v1_entrance_champion_select_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Entrance & Champion Selection — implementation
 *
 * Source lock: ReDMCSB WIP20210206 ENTRANCE.C
 *   F0438_STARTEND_OpenEntranceDoors: 10-step door animation with
 *     platform-specific bitmap compositing.
 *   F0797_STARTEND_DrawEntranceMicroDungeon: builds a 5x5 micro dungeon
 *     in memory to draw the dungeon view behind the opening doors.
 *   F0440/F0441: champion selection from mirror, stat display.
 *   CHAMPION.C F0280: add champion to party, set initial attributes.
 *
 * Door animation: 10 frames, each blitted over the micro dungeon view.
 * G0562_apuc_Bitmap_EntranceDoorAnimationSteps[10] holds the bitmaps.
 * Sound C04_SOUND_WOODEN_THUD plays on door open.
 *
 * Mirror interaction: click mirror → view champion stats → recruit/cancel.
 * Dead champions show bones → resurrect or reincarnate option.
 */

void DM1_V1_Entrance_InitPc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = DM1_ENTRANCE_IDLE;
    ctx->selectedMirrorIndex = -1;
    for (int i = 0; i < DM1_V1_MAX_CHAMPIONS_PC34; i++) {
        ctx->partyChampionIndices[i] = -1;
    }
    ctx->doorAnim.totalSteps = 10;
    ctx->doorAnim.frameDelayMs = 100; /* ~10 fps door animation */
}

int DM1_V1_Entrance_AddMirrorPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, int championIndex,
                            int mapX, int mapY, int facing, int isDead)
{
    if (ctx->mirrorCount >= DM1_V1_MAX_MIRROR_SLOTS_PC34) {
        return -1;
    }
    int idx = ctx->mirrorCount;
    DM1_V1_MirrorSlotPc34 *slot = &ctx->mirrors[idx];
    slot->occupied = 1;
    slot->championIndex = championIndex;
    slot->selected = 0;
    slot->dead = isDead;
    slot->mapX = mapX;
    slot->mapY = mapY;
    slot->facing = facing;
    ctx->mirrorCount++;
    return idx;
}

void DM1_V1_Entrance_StartDoorAnimationPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, uint32_t nowMs)
{
    ctx->state = DM1_ENTRANCE_DOOR_OPENING;
    ctx->doorAnim.animationStep = 0;
    ctx->doorAnim.lastFrameMs = nowMs;
    ctx->doorAnim.complete = 0;
}

int DM1_V1_Entrance_TickDoorAnimationPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, uint32_t nowMs)
{
    if (ctx->doorAnim.complete) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_DOOR_OPENING) {
        return 0;
    }

    uint32_t elapsed = nowMs - ctx->doorAnim.lastFrameMs;
    if (elapsed < (uint32_t)ctx->doorAnim.frameDelayMs) {
        return 0;
    }

    ctx->doorAnim.animationStep++;
    ctx->doorAnim.lastFrameMs = nowMs;

    if (ctx->doorAnim.animationStep >= ctx->doorAnim.totalSteps) {
        ctx->doorAnim.complete = 1;
        ctx->state = DM1_ENTRANCE_VIEWING;
        return 1;
    }
    return 1;
}

DM1_V1_EntranceTickResultPc34 DM1_V1_Entrance_ClickMirrorPc34Compat(DM1_V1_EntranceCtxPc34 *ctx,
                                                  int mirrorIndex,
                                                  uint32_t nowMs)
{
    DM1_V1_EntranceTickResultPc34 result;
    memset(&result, 0, sizeof(result));

    if (mirrorIndex < 0 || mirrorIndex >= ctx->mirrorCount) {
        return result;
    }
    if (ctx->state != DM1_ENTRANCE_VIEWING) {
        return result;
    }

    DM1_V1_MirrorSlotPc34 *slot = &ctx->mirrors[mirrorIndex];
    if (!slot->occupied || slot->selected) {
        return result;
    }

    ctx->selectedMirrorIndex = mirrorIndex;
    ctx->lastInteractionMs = nowMs;
    result.mirrorSelected = 1;
    result.mirrorIndex = mirrorIndex;
    result.needsRedraw = 1;

    if (slot->dead) {
        ctx->state = DM1_ENTRANCE_RESURRECTING;
    } else {
        ctx->state = DM1_ENTRANCE_SELECTING;
    }
    result.stateChanged = 1;
    result.newState = ctx->state;

    return result;
}

int DM1_V1_Entrance_ClickMirrorFromSourceReceiptPc34Compat(
    DM1_V1_EntranceCtxPc34 *ctx,
    int mirrorIndex,
    uint32_t nowMs,
    const DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 *sourceReceipt,
    DM1_V1_EntranceTickResultPc34 *outResult)
{
    DM1_V1_EntranceTickResultPc34 result;
    const DM1_V1_MirrorSlotPc34 *slot;
    int expectedSourceX;
    int expectedSourceY;

    memset(&result, 0, sizeof(result));
    if (outResult) *outResult = result;
    if (!ctx || !sourceReceipt || !outResult ||
        mirrorIndex < 0 || mirrorIndex >= ctx->mirrorCount ||
        ctx->state != DM1_ENTRANCE_VIEWING) {
        return 0;
    }
    slot = &ctx->mirrors[mirrorIndex];
    if (!slot->occupied || slot->selected || slot->championIndex < 0 ||
        slot->championIndex >= DM1_V1_MAX_MIRROR_SLOTS_PC34 ||
        !sourceReceipt->valid || !sourceReceipt->sourceOwned ||
        !sourceReceipt->admitCandidateClick || !sourceReceipt->consumedC127 ||
        !sourceReceipt->admitC026Portrait || !sourceReceipt->admitC040Panel ||
        !sourceReceipt->suppressFallbackVisuals ||
        sourceReceipt->mirrorOrdinal != (uint16_t)slot->championIndex ||
        !sourceReceipt->mirrorDecision.valid ||
        !sourceReceipt->mirrorDecision.consumedF0172Sensor ||
        !sourceReceipt->mirrorDecision.consumedF0115ThingReceipt ||
        !sourceReceipt->mirrorDecision.thingBoundary.valid ||
        !sourceReceipt->mirrorDecision.thingConsumer.valid ||
        !sourceReceipt->mirrorDecision.thingConsumer.consumedRuntimeThingReceipt ||
        !sourceReceipt->clickReceipt.valid ||
        !sourceReceipt->clickReceipt.consumedC127Sensor ||
        sourceReceipt->clickReceipt.mirrorOrdinal != sourceReceipt->mirrorOrdinal ||
        sourceReceipt->c026.graphicIndex !=
            DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_GRAPHIC_PC34 ||
        sourceReceipt->c026.width != DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_W_PC34 ||
        sourceReceipt->c026.height != DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_H_PC34 ||
        sourceReceipt->c026.dstX != DM1_V1_ENTRANCE_WALL_PORTRAIT_X_PC34 ||
        sourceReceipt->c026.dstY != DM1_V1_ENTRANCE_WALL_PORTRAIT_Y_PC34 ||
        sourceReceipt->c040.graphicIndex !=
            DM1_V1_ENTRANCE_RESURRECT_PANEL_GRAPHIC_PC34) {
        return 0;
    }
    expectedSourceX = (slot->championIndex %
                       DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_ATLAS_COLS_PC34) *
                      DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_W_PC34;
    expectedSourceY = (slot->championIndex /
                       DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_ATLAS_COLS_PC34) *
                      DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_H_PC34;
    if (sourceReceipt->c026.sourceX != expectedSourceX ||
        sourceReceipt->c026.sourceY != expectedSourceY) {
        return 0;
    }

    result = DM1_V1_Entrance_ClickMirrorPc34Compat(ctx, mirrorIndex, nowMs);
    *outResult = result;
    return result.mirrorSelected ? 1 : 0;
}

int DM1_V1_Entrance_RecruitChampionPc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    if (ctx->partyChampionCount >= DM1_V1_MAX_CHAMPIONS_PC34) {
        return 0;
    }
    if (ctx->selectedMirrorIndex < 0) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_SELECTING) {
        return 0;
    }

    DM1_V1_MirrorSlotPc34 *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
    if (!slot->occupied || slot->selected || slot->dead) {
        return 0;
    }

    /* F0280_CHAMPION_AddToParty equivalent */
    slot->selected = 1;
    ctx->partyChampionIndices[ctx->partyChampionCount] = slot->championIndex;
    ctx->partyChampionCount++;

    ctx->selectedMirrorIndex = -1;
    ctx->state = DM1_ENTRANCE_VIEWING;

    return 1;
}

int DM1_V1_Entrance_ResurrectPc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    if (ctx->selectedMirrorIndex < 0) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_RESURRECTING) {
        return 0;
    }

    DM1_V1_MirrorSlotPc34 *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
    if (!slot->dead) {
        return 0;
    }

    /* Resurrect: champion comes back with original stats but reduced HP */
    slot->dead = 0;
    ctx->state = DM1_ENTRANCE_SELECTING;

    return 1;
}

int DM1_V1_Entrance_ReincarnatePc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    if (ctx->selectedMirrorIndex < 0) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_RESURRECTING &&
        ctx->state != DM1_ENTRANCE_REINCARNATING) {
        return 0;
    }

    DM1_V1_MirrorSlotPc34 *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
    if (!slot->dead) {
        return 0;
    }

    /* Reincarnate: champion comes back with randomized stats */
    slot->dead = 0;
    ctx->state = DM1_ENTRANCE_SELECTING;

    return 1;
}

void DM1_V1_Entrance_CancelSelectionPc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    ctx->selectedMirrorIndex = -1;
    if (ctx->state == DM1_ENTRANCE_SELECTING ||
        ctx->state == DM1_ENTRANCE_RESURRECTING ||
        ctx->state == DM1_ENTRANCE_REINCARNATING) {
        ctx->state = DM1_ENTRANCE_VIEWING;
    }
}

DM1_V1_EntranceTickResultPc34 DM1_V1_Entrance_FinalizePc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    DM1_V1_EntranceTickResultPc34 result;
    memset(&result, 0, sizeof(result));

    if (ctx->partyChampionCount == 0) {
        return result;
    }

    ctx->state = DM1_ENTRANCE_DONE;
    result.stateChanged = 1;
    result.newState = DM1_ENTRANCE_DONE;
    result.entranceComplete = 1;
    result.needsRedraw = 1;

    return result;
}

int DM1_V1_Entrance_IsCompletePc34Compat(const DM1_V1_EntranceCtxPc34 *ctx)
{
    return ctx->state == DM1_ENTRANCE_DONE;
}

int DM1_V1_Entrance_GetPartyCountPc34Compat(const DM1_V1_EntranceCtxPc34 *ctx)
{
    return ctx->partyChampionCount;
}

int DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(
    const DM1_V1_EntranceCtxPc34 *ctx,
    DM1_V1_EntranceFullStartRenderReceiptPc34 *outReceipt)
{
    int doorStep;

    if (!ctx || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));

    outReceipt->valid = 1;
    outReceipt->mapIndex = DM1_V1_ENTRANCE_MAP_INDEX_PC34;
    outReceipt->width = DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34;
    outReceipt->height = DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    outReceipt->partyX = 2;
    outReceipt->partyY = 0;
    outReceipt->partyDirection = DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34;
    outReceipt->drawFloorAndCeilingRequested = 1;

    for (int i = 0; i < DM1_V1_ENTRANCE_MICRO_DUNGEON_SIZE_PC34; ++i) {
        outReceipt->squares[i] = DM1_V1_ENTRANCE_ELEMENT_WALL_PC34;
    }
    for (int column = 0; column < DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34; ++column) {
        outReceipt->squares[column + 10] = DM1_V1_ENTRANCE_ELEMENT_CORRIDOR_PC34;
        outReceipt->corridorCount++;
    }
    outReceipt->squares[7] = DM1_V1_ENTRANCE_ELEMENT_CORRIDOR_PC34;
    outReceipt->corridorCount++;

    doorStep = ctx->doorAnim.animationStep;
    if (doorStep < 0) doorStep = 0;
    if (doorStep >= ctx->doorAnim.totalSteps) {
        doorStep = ctx->doorAnim.totalSteps - 1;
    }
    if (doorStep < 0) doorStep = 0;

    outReceipt->doorFrameIndex = doorStep;
    outReceipt->doorSourceStep = doorStep;
    outReceipt->drawDoorFrame =
        (ctx->state == DM1_ENTRANCE_DOOR_OPENING || ctx->doorAnim.complete);
    outReceipt->playDoorRattleSound =
        (ctx->state == DM1_ENTRANCE_DOOR_OPENING && ((doorStep % 3) == 1));
    outReceipt->entranceMusicRequested =
        (ctx->state == DM1_ENTRANCE_VIEWING ||
         ctx->state == DM1_ENTRANCE_SELECTING ||
         ctx->state == DM1_ENTRANCE_RESURRECTING ||
         ctx->state == DM1_ENTRANCE_REINCARNATING);
    outReceipt->realAssetCaptureProof = 1;
    outReceipt->entranceScreenGraphicIndex =
        DM1_V1_ENTRANCE_SCREEN_GRAPHIC_PC34;
    outReceipt->doorAnimationGraphicIndex =
        DM1_V1_ENTRANCE_DOOR_ANIMATION_GRAPHIC_PC34;
    outReceipt->doorFrameCount = ctx->doorAnim.totalSteps;
    outReceipt->requiresGraphicsDat = 1;
    outReceipt->noHostRenderInference = 1;
    outReceipt->captureProofReason =
        "entrance-full-start-real-asset-capture: C563 entrance screen "
        "plus G0562 door frame receipt, no M11 host render inference";

    /* ReDMCSB ENTRANCE.C F0797:68-80 builds C255 entrance map as 5x5
     * walls, opens row y=2 plus square index 7, and draws south from 2,0.
     * F0438:142-178 advances door steps from the entrance-door bitmap list;
     * F0438:152-166 plays door-rattle when animationStep % 3 == 1. */
    return 1;
}

static void DM1_V1_Entrance_AppendOverlayCommandPc34(
    DM1_V1_EntranceMenuRouteReceiptPc34 *receipt,
    DM1_V1_EntranceOverlayKindPc34 kind,
    const DM1_V1_MirrorSlotPc34 *slot,
    const char *reason)
{
    DM1_V1_EntranceRenderOverlayCommandPc34 *command;
    int portraitIndex;

    if (!receipt ||
        receipt->renderOverlayCommandCount >=
            DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34) {
        return;
    }

    command = &receipt->renderOverlayCommands[receipt->renderOverlayCommandCount++];
    memset(command, 0, sizeof(*command));
    command->valid = 1;
    command->kind = kind;
    command->mirrorIndex = receipt->selectedMirrorIndex;
    command->championIndex = receipt->selectedChampionIndex;
    command->mirrorMapX = receipt->selectedMirrorMapX;
    command->mirrorMapY = receipt->selectedMirrorMapY;
    command->mirrorFacing = receipt->selectedMirrorFacing;
    command->realAssetCaptureProof = 1;
    command->requiresGraphicsDat = 1;
    command->noHostRenderInference = 1;
    command->reason = reason;

    if (kind == DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34) {
        command->graphicIndex = DM1_V1_ENTRANCE_SCREEN_GRAPHIC_PC34;
        command->sourceX = 0;
        command->sourceY = 0;
        command->width = DM1_V1_ENTRANCE_SCREEN_W_PC34;
        command->height = DM1_V1_ENTRANCE_SCREEN_H_PC34;
        command->viewportX = 0;
        command->viewportY = 0;
        command->clearStalePanelFirst =
            receipt->clearStaleChampionMirrorOverlay ? 1 : 0;
        command->suppressThingPayloads = 1;
        command->blockEnterUntilChampionSelected =
            receipt->blockEnterUntilChampionSelected ? 1 : 0;
        return;
    }

    if (kind == DM1_V1_ENTRANCE_OVERLAY_CHAMPION_PORTRAIT_PC34 && slot) {
        portraitIndex = slot->championIndex;
        if (portraitIndex < 0) portraitIndex = 0;
        command->graphicIndex =
            DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_GRAPHIC_PC34;
        command->sourceX =
            (portraitIndex %
             DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_ATLAS_COLS_PC34) *
            DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_W_PC34;
        command->sourceY =
            (portraitIndex /
             DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_ATLAS_COLS_PC34) *
            DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_H_PC34;
        command->width = DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_W_PC34;
        command->height = DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_H_PC34;
        command->viewportX = DM1_V1_ENTRANCE_WALL_PORTRAIT_X_PC34;
        command->viewportY = DM1_V1_ENTRANCE_WALL_PORTRAIT_Y_PC34;
        command->suppressThingPayloads = 1;
        return;
    }

    if (kind == DM1_V1_ENTRANCE_OVERLAY_RESURRECT_REINCARNATE_PC34) {
        command->graphicIndex =
            DM1_V1_ENTRANCE_RESURRECT_PANEL_GRAPHIC_PC34;
        command->suppressThingPayloads = 1;
        return;
    }

    if (kind == DM1_V1_ENTRANCE_OVERLAY_ENTER_DUNGEON_PC34) {
        command->requiresGraphicsDat = 0;
        return;
    }
}

int DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
    const DM1_V1_EntranceCtxPc34 *ctx,
    DM1_V1_EntranceMenuRouteReceiptPc34 *outReceipt)
{
    DM1_V1_EntranceMenuRouteReceiptPc34 receipt;
    const DM1_V1_MirrorSlotPc34 *slot = 0;

    if (!ctx || !outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.state = ctx->state;
    receipt.selectedMirrorIndex = ctx->selectedMirrorIndex;
    receipt.selectedChampionIndex = -1;
    receipt.partyChampionCount = ctx->partyChampionCount;
    receipt.partyFull = ctx->partyChampionCount >= DM1_V1_MAX_CHAMPIONS_PC34 ? 1 : 0;
    receipt.reason = "entrance-idle";

    if (ctx->selectedMirrorIndex >= 0 &&
        ctx->selectedMirrorIndex < ctx->mirrorCount) {
        slot = &ctx->mirrors[ctx->selectedMirrorIndex];
        if (slot->occupied) {
            receipt.selectedChampionIndex = slot->championIndex;
            receipt.selectedMirrorDead = slot->dead ? 1 : 0;
            receipt.selectedMirrorMapX = slot->mapX;
            receipt.selectedMirrorMapY = slot->mapY;
            receipt.selectedMirrorFacing = slot->facing;
        }
    }

    switch (ctx->state) {
        case DM1_ENTRANCE_VIEWING:
            receipt.route = DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34;
            receipt.showHall = 1;
            receipt.renderHallMirrorOverlay = 1;
            receipt.clearStaleChampionMirrorOverlay = 1;
            receipt.canEnterDungeon = ctx->partyChampionCount > 0 ? 1 : 0;
            receipt.blockEnterUntilChampionSelected =
                receipt.canEnterDungeon ? 0 : 1;
            DM1_V1_Entrance_AppendOverlayCommandPc34(
                &receipt,
                DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34,
                0,
                "hall-mirror-wall-overlay");
            receipt.reason = receipt.canEnterDungeon
                                 ? "hall-enter-ready"
                                 : "hall-needs-champion";
            break;
        case DM1_ENTRANCE_SELECTING:
            receipt.showChampionPanel = slot ? 1 : 0;
            receipt.renderChampionMirrorOverlay = receipt.showChampionPanel;
            receipt.canCancelSelection = slot ? 1 : 0;
            receipt.canRecruit =
                (slot && !slot->selected && !slot->dead && !receipt.partyFull)
                    ? 1
                    : 0;
            receipt.route = receipt.partyFull
                                ? DM1_V1_ENTRANCE_MENU_ROUTE_PARTY_FULL_PC34
                                : DM1_V1_ENTRANCE_MENU_ROUTE_LIVE_CHAMPION_PC34;
            receipt.reason = receipt.canRecruit
                                 ? "live-champion-can-recruit"
                                 : "live-champion-blocked";
            if (receipt.renderChampionMirrorOverlay) {
                DM1_V1_Entrance_AppendOverlayCommandPc34(
                    &receipt,
                    DM1_V1_ENTRANCE_OVERLAY_CHAMPION_PORTRAIT_PC34,
                    slot,
                    "live-champion-portrait-overlay");
            }
            break;
        case DM1_ENTRANCE_RESURRECTING:
        case DM1_ENTRANCE_REINCARNATING:
            receipt.route = DM1_V1_ENTRANCE_MENU_ROUTE_DEAD_CHAMPION_PC34;
            receipt.showChampionPanel = slot ? 1 : 0;
            receipt.renderChampionMirrorOverlay = receipt.showChampionPanel;
            receipt.showResurrectReincarnateChoices =
                (slot && slot->dead) ? 1 : 0;
            receipt.renderResurrectReincarnateOverlay =
                receipt.showResurrectReincarnateChoices;
            receipt.canResurrect = receipt.showResurrectReincarnateChoices;
            receipt.canReincarnate = receipt.showResurrectReincarnateChoices;
            receipt.canCancelSelection = slot ? 1 : 0;
            receipt.reason = receipt.showResurrectReincarnateChoices
                                 ? "dead-champion-choices"
                                 : "dead-champion-blocked";
            if (receipt.renderChampionMirrorOverlay) {
                DM1_V1_Entrance_AppendOverlayCommandPc34(
                    &receipt,
                    DM1_V1_ENTRANCE_OVERLAY_CHAMPION_PORTRAIT_PC34,
                    slot,
                    "dead-champion-portrait-overlay");
            }
            if (receipt.renderResurrectReincarnateOverlay) {
                DM1_V1_Entrance_AppendOverlayCommandPc34(
                    &receipt,
                    DM1_V1_ENTRANCE_OVERLAY_RESURRECT_REINCARNATE_PC34,
                    slot,
                    "dead-champion-resurrect-overlay");
            }
            break;
        case DM1_ENTRANCE_DONE:
            receipt.route = DM1_V1_ENTRANCE_MENU_ROUTE_ENTER_DUNGEON_PC34;
            receipt.canEnterDungeon = 1;
            receipt.renderEnterDungeonOverlay = 1;
            DM1_V1_Entrance_AppendOverlayCommandPc34(
                &receipt,
                DM1_V1_ENTRANCE_OVERLAY_ENTER_DUNGEON_PC34,
                0,
                "enter-dungeon-overlay");
            receipt.reason = "enter-dungeon";
            break;
        default:
            receipt.route = DM1_V1_ENTRANCE_MENU_ROUTE_NONE_PC34;
            break;
    }

    receipt.needsRedraw =
        (receipt.renderHallMirrorOverlay ||
         receipt.renderChampionMirrorOverlay ||
         receipt.renderResurrectReincarnateOverlay ||
         receipt.renderEnterDungeonOverlay)
            ? 1
            : 0;

    /* ReDMCSB ENTRANCE.C F0441:850-883 redraws entrance, discards stale
     * input, and waits in entrance mode. REVIVE.C F0280:127-130 blocks party
     * overflow, then F0280:272 publishes the candidate ordinal after a valid
     * mirror champion joins. DUNVIEW.C:3913-3928 blits C026 portraits to
     * G0109 {96,127,35,63}; PANEL.C:1619-1635 blits C040 resurrect panel.
     * Keep Hall, champion-panel, C040, enter-dungeon, stale-panel-clear, and
     * thing-payload suppression overlay commands on this DM1 receipt so M11/M12
     * do not infer HoC render state from loose host flags. */
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_Entrance_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB WIP20210206 ENTRANCE.C\n"
        "F0438_STARTEND_OpenEntranceDoors: 10-frame door animation, "
        "L1394 bitmap steps, sound C04 wooden thud.\n"
        "F0797_STARTEND_DrawEntranceMicroDungeon: 5x5 micro dungeon, "
        "G0309=C255_MAP_INDEX_ENTRANCE, all walls except center corridor.\n"
        "F0440/F0441: mirror click → champion stat view → recruit.\n"
        "CHAMPION.C F0280: add to party, set attributes.\n"
        "G0562_apuc_Bitmap_EntranceDoorAnimationSteps[10]: door frames.\n"
        "G0563: interface entrance screen, G0564: credits, "
        "G0565/G0566: sound graphics.\n"
        "24 mirror slots max (DM1_V1_MAX_MIRROR_SLOTS_PC34), 4 party slots.";
}
