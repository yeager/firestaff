#include "dm1_v1_game_loop_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Game Loop Orchestrator — implementation
 *
 * Source lock: ReDMCSB WIP20210206 GAMELOOP.C
 *   F0002_MAIN_GameLoop_CPSDF — the infinite for(;;) loop.
 *   F0003_MAIN_ProcessNewPartyMap_CPSE — map transition.
 *
 * Tick order (from F0002):
 *   1. G0317 = 0 (reset vblank wait count)
 *   2. If G0327 != -1: F0003 + F0267 move result + G0327 = -1 + discard input
 *   3. F0261_TIMELINE_Process
 *   4. If G0327 != -1 again (timeline caused map change): goto step 2
 *   5. Music update (platform-specific)
 *   6. If !G0300 (not resting) and !G0423 (no inventory): F0128 dungeon view draw
 *   7. Mouse pointer updates (G0325, G0326)
 *   8. F0363 command highlight disable
 *   9. F0065 sound play pending
 *  10. F0320 apply damage/wounds
 *  11. If G0303 (party dead): break
 *  12. Input wait loop (vblank count < max, check G0321, process commands)
 */

void m11_game_loop_init(M11_GameLoopState *state, int extendedVBlankWait)
{
    memset(state, 0, sizeof(*state));
    state->gameState = DM1_GAME_UNINIT;
    state->waitForInputMaxVBlankCount = extendedVBlankWait
        ? M11_WAIT_FOR_INPUT_MAX_VBLANK_COUNT_EXTENDED
        : M11_WAIT_FOR_INPUT_MAX_VBLANK_COUNT_DEFAULT;
    state->newPartyMapIndex = -1;
}

M11_GameLoopTickResult m11_game_loop_tick(M11_GameLoopState *state, uint32_t nowMs)
{
    M11_GameLoopTickResult result;
    memset(&result, 0, sizeof(result));
    result.newMapIndex = -1;

    /* Step 1: Reset vblank wait count (G0317 = 0) */
    state->waitForInputVBlankCount = 0;

    /* Step 2: Process new party map if pending (F0003 + F0267) */
    if (state->newPartyMapIndex != -1) {
        result.newMapProcessed = 1;
        result.newMapIndex = state->newPartyMapIndex;
        /*
         * Caller must: F0003(newMapIndex), F0267(PARTY, -1, 0, partyX, partyY),
         * then discard all input (F0357).
         */
        state->newPartyMapIndex = -1;
        state->stopWaitingForInput = 0;
    }
    result.lastPhaseCompleted = DM1_PHASE_NEW_MAP;

    /* Step 3: Timeline processing (F0261) — caller invokes timeline */
    result.timelineEventsProcessed = 1; /* flag: caller should call timeline */
    result.lastPhaseCompleted = DM1_PHASE_TIMELINE;

    /*
     * Step 4: If timeline caused a new map (G0327 != -1), the caller
     * should detect result.newMapProcessed==0 but state->newPartyMapIndex!=-1
     * and re-call tick or handle inline. We set the flag for the caller.
     */

    /* Step 6: Dungeon view draw (F0128) */
    if (!state->partyResting) {
        if (!state->inventoryChampionOrdinal) {
            result.dungeonViewDrawn = 1;
        }
        result.inventoryOpen = (state->inventoryChampionOrdinal != 0);
    }
    result.partyResting = state->partyResting;
    result.lastPhaseCompleted = DM1_PHASE_DUNGEON_VIEW_DRAW;

    /* Step 7: Mouse pointer updates (G0325, G0326) */
    if (state->setMousePointerToObject) {
        result.mousePointerUpdated = 1;
        state->setMousePointerToObject = 0;
    }
    if (state->refreshMousePointer) {
        result.mousePointerUpdated = 1;
        state->refreshMousePointer = 0;
    }
    result.lastPhaseCompleted = DM1_PHASE_MOUSE_POINTER_UPDATE;

    /* Step 8: Command highlight disable (F0363) */
    result.commandHighlightDisabled = 1;
    result.lastPhaseCompleted = DM1_PHASE_COMMAND_HIGHLIGHT;

    /* Step 9: Sound play pending (F0065) */
    result.soundPlayed = 1;
    result.lastPhaseCompleted = DM1_PHASE_SOUND;

    /* Step 10: Apply damage and wounds (F0320) */
    result.damageApplied = 1;
    result.lastPhaseCompleted = DM1_PHASE_DAMAGE_WOUNDS;

    /* Step 11: Death check (G0303) */
    result.partyDead = state->partyDead;
    result.lastPhaseCompleted = DM1_PHASE_DEATH_CHECK;

    /* Step 12: Input wait — set vblank count and stop flag */
    result.stopWaitingForInput = state->stopWaitingForInput;
    result.vblankWaitCount = state->waitForInputVBlankCount;
    result.lastPhaseCompleted = DM1_PHASE_INPUT_WAIT;

    /* Exit check */
    result.exitRequested = state->exitGameImmediately;

    /* Update tick tracking */
    state->tickCount++;
    state->lastTickMs = nowMs;

    return result;
}

void m11_game_loop_request_new_map(M11_GameLoopState *state, int newMapIndex)
{
    state->newPartyMapIndex = newMapIndex;
}

void m11_game_loop_set_party_dead(M11_GameLoopState *state)
{
    state->partyDead = 1;
    state->gameState = DM1_GAME_DEATH;
}

void m11_game_loop_set_inventory(M11_GameLoopState *state, int championOrdinal)
{
    state->inventoryChampionOrdinal = championOrdinal;
    if (championOrdinal) {
        state->gameState = DM1_GAME_INVENTORY;
    } else if (state->gameState == DM1_GAME_INVENTORY) {
        state->gameState = DM1_GAME_PLAYING;
    }
}

void m11_game_loop_set_resting(M11_GameLoopState *state, int resting)
{
    state->partyResting = resting;
}

void m11_game_loop_request_exit(M11_GameLoopState *state)
{
    state->exitGameImmediately = 1;
}

int m11_game_loop_should_continue(const M11_GameLoopState *state)
{
    return !state->partyDead && !state->exitGameImmediately;
}

const char *m11_game_loop_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206 GAMELOOP.C\n"
        "F0002_MAIN_GameLoop_CPSDF: infinite loop — reset G0317, "
        "check G0327 (new map), F0261 timeline, F0128 dungeon view, "
        "F0363 command highlight, F0065 sound, F0320 damage/wounds, "
        "G0303 death check, input wait loop with G0318 max vblank.\n"
        "F0003_MAIN_ProcessNewPartyMap_CPSE: map transition + F0267 move + discard input.\n"
        "G0317=vblank wait count, G0318=max(10 or 12), G0321=stop waiting, "
        "G0300=party resting, G0423=inventory champion ordinal, "
        "G0303=party dead, G0327=new party map index, G2151=exit game.";
}
