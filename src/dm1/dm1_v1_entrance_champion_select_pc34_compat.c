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
    for (int i = 0; i < M11_MAX_CHAMPIONS; i++) {
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

int DM1_V1_Entrance_RecruitChampionPc34Compat(DM1_V1_EntranceCtxPc34 *ctx)
{
    if (ctx->partyChampionCount >= M11_MAX_CHAMPIONS) {
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
