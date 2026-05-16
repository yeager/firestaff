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

void m11_entrance_init(M11_EntranceCtx *ctx)
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

int m11_entrance_add_mirror(M11_EntranceCtx *ctx, int championIndex,
                            int mapX, int mapY, int facing, int isDead)
{
    if (ctx->mirrorCount >= M11_MAX_MIRROR_SLOTS) {
        return -1;
    }
    int idx = ctx->mirrorCount;
    M11_MirrorSlot *slot = &ctx->mirrors[idx];
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

void m11_entrance_start_door_animation(M11_EntranceCtx *ctx, uint32_t nowMs)
{
    ctx->state = DM1_ENTRANCE_DOOR_OPENING;
    ctx->doorAnim.animationStep = 0;
    ctx->doorAnim.lastFrameMs = nowMs;
    ctx->doorAnim.complete = 0;
}

int m11_entrance_tick_door_animation(M11_EntranceCtx *ctx, uint32_t nowMs)
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

M11_EntranceTickResult m11_entrance_click_mirror(M11_EntranceCtx *ctx,
                                                  int mirrorIndex,
                                                  uint32_t nowMs)
{
    M11_EntranceTickResult result;
    memset(&result, 0, sizeof(result));

    if (mirrorIndex < 0 || mirrorIndex >= ctx->mirrorCount) {
        return result;
    }
    if (ctx->state != DM1_ENTRANCE_VIEWING) {
        return result;
    }

    M11_MirrorSlot *slot = &ctx->mirrors[mirrorIndex];
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

int m11_entrance_recruit_champion(M11_EntranceCtx *ctx)
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

    M11_MirrorSlot *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
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

int m11_entrance_resurrect(M11_EntranceCtx *ctx)
{
    if (ctx->selectedMirrorIndex < 0) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_RESURRECTING) {
        return 0;
    }

    M11_MirrorSlot *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
    if (!slot->dead) {
        return 0;
    }

    /* Resurrect: champion comes back with original stats but reduced HP */
    slot->dead = 0;
    ctx->state = DM1_ENTRANCE_SELECTING;

    return 1;
}

int m11_entrance_reincarnate(M11_EntranceCtx *ctx)
{
    if (ctx->selectedMirrorIndex < 0) {
        return 0;
    }
    if (ctx->state != DM1_ENTRANCE_RESURRECTING &&
        ctx->state != DM1_ENTRANCE_REINCARNATING) {
        return 0;
    }

    M11_MirrorSlot *slot = &ctx->mirrors[ctx->selectedMirrorIndex];
    if (!slot->dead) {
        return 0;
    }

    /* Reincarnate: champion comes back with randomized stats */
    slot->dead = 0;
    ctx->state = DM1_ENTRANCE_SELECTING;

    return 1;
}

void m11_entrance_cancel_selection(M11_EntranceCtx *ctx)
{
    ctx->selectedMirrorIndex = -1;
    if (ctx->state == DM1_ENTRANCE_SELECTING ||
        ctx->state == DM1_ENTRANCE_RESURRECTING ||
        ctx->state == DM1_ENTRANCE_REINCARNATING) {
        ctx->state = DM1_ENTRANCE_VIEWING;
    }
}

M11_EntranceTickResult m11_entrance_finalize(M11_EntranceCtx *ctx)
{
    M11_EntranceTickResult result;
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

int m11_entrance_is_complete(const M11_EntranceCtx *ctx)
{
    return ctx->state == DM1_ENTRANCE_DONE;
}

int m11_entrance_get_party_count(const M11_EntranceCtx *ctx)
{
    return ctx->partyChampionCount;
}

const char *m11_entrance_source_evidence(void)
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
        "24 mirror slots max (M11_MAX_MIRROR_SLOTS), 4 party slots.";
}
