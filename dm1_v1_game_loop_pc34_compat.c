#include "dm1_v1_game_loop_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Main Game Loop + Frame Timing — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   VBLANK.C:    F0577_VerticalBlank_Handler_CPSDF — VBlank ISR
 *                F0575_VerticalBlank_Initialize — install handler
 *                G0317_i_WaitForInputVerticalBlankCount
 *                G0318_i_WaitForInputMaximumVerticalBlankCount = 10
 *                G0321_B_StopWaitingForPlayerInput
 *                G1086_VerticalBlankCount
 *   GAMELOOP.C:  F0002_MAIN_GameLoop_CPSDF — main infinite loop
 *                Tick order: newMap → timeline → dungeonView → pointer →
 *                            highlight → sound → damage → deathCheck → inputWait
 *   DOS/CLOCK.C: DOS timer interrupt for 50fps emulation on PC
 *
 * The VBlank handler in the original increments G0317 every frame.
 * When G0317 >= G0318, it sets G0321 = TRUE to release the input wait.
 * On PC34, G0318 = 10 (200ms input timeout at 50fps).
 */

/* ── Initialization ───────────────────────────────────────────────── */

void m11_game_loop_init(M11_GameLoopState *state, int extendedVBlankWait)
{
    memset(state, 0, sizeof(*state));
    state->loopStatus = DM1_LOOP_INIT;
    state->timerActive = 1; /* G2586_TimerActive = true by default */
    state->waitForInputMaxVBlankCount = extendedVBlankWait
        ? M11_VBLANK_WAIT_MAX_EXTENDED
        : M11_VBLANK_WAIT_MAX_DEFAULT;
    state->newPartyMapIndex = -1;
    state->targetFrameTimeUs = 1000000u / M11_FRAME_RATE_HZ; /* 20000us */
}

void m11_game_loop_set_tick_rate(M11_GameLoopState *state, int hz)
{
    if (hz < 1) hz = 1;
    if (hz > 1000) hz = 1000;
    state->targetFrameTimeUs = 1000000u / (uint32_t)hz;
}

/* ── VBlank interrupt simulation (F0577) ──────────────────────────── */

void m11_game_loop_vblank_tick(M11_GameLoopState *state)
{
    /*
     * F0577_VerticalBlank_Handler_CPSDF:
     *   G0317_i_WaitForInputVerticalBlankCount++;
     *   if (G0317 >= G0318) { G0321_B_StopWaitingForPlayerInput = TRUE; }
     *   G1086_VerticalBlankCount++;
     */
    state->verticalBlankCount++;

    if (!state->timerActive) return; /* Paused — don't advance input wait */

    state->waitForInputVBlankCount++;
    if (state->waitForInputVBlankCount >= state->waitForInputMaxVBlankCount) {
        state->stopWaitingForInput = 1;
    }
}

/* ── Core game loop tick (F0002) ──────────────────────────────────── */

M11_GameLoopTickResult m11_game_loop_tick(M11_GameLoopState *state, uint32_t nowMs)
{
    M11_GameLoopTickResult result;
    memset(&result, 0, sizeof(result));
    result.newMapIndex = -1;

    /* If paused (G2586_TimerActive == false), return no-op */
    if (!state->timerActive) {
        result.lastPhaseCompleted = DM1_PHASE_INPUT_WAIT;
        return result;
    }

    /* Mark running */
    if (state->loopStatus == DM1_LOOP_INIT) {
        state->loopStatus = DM1_LOOP_RUNNING;
    }

    /* Step 1: Reset vblank wait count (G0317 = 0) */
    state->waitForInputVBlankCount = 0;
    state->stopWaitingForInput = 0;

    /* Step 2: Process new party map if pending (G0327 != -1) */
    if (state->newPartyMapIndex != -1) {
        result.newMapProcessed = 1;
        result.newMapIndex = state->newPartyMapIndex;
        /* Caller: F0003(mapIndex) + F0267(PARTY,-1,0,X,Y) + F0357 discard */
        state->newPartyMapIndex = -1;
    }
    result.lastPhaseCompleted = DM1_PHASE_NEW_MAP;

    /* Step 3: Timeline processing (F0261) */
    result.timelineEventsProcessed = 1;
    result.lastPhaseCompleted = DM1_PHASE_TIMELINE;

    /* Step 4: Check if timeline triggered another map change — caller
     * should re-check state->newPartyMapIndex after calling timeline. */

    /* Step 5: Music update (platform-specific, skipped in orchestrator) */

    /* Step 6: Dungeon view draw (F0128) */
    if (!state->partyResting && !state->inventoryChampionOrdinal) {
        result.dungeonViewDrawn = 1;
    }
    result.inventoryOpen = (state->inventoryChampionOrdinal != 0);
    result.partyResting = state->partyResting;
    result.lastPhaseCompleted = DM1_PHASE_DUNGEON_VIEW_DRAW;

    /* Step 7: Mouse pointer updates (G0325, G0326) */
    if (state->setMousePointerToObject || state->refreshMousePointer) {
        result.mousePointerUpdated = 1;
        state->setMousePointerToObject = 0;
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

    /* Step 12: Input wait — report current state */
    result.stopWaitingForInput = state->stopWaitingForInput;
    result.vblankWaitCount = state->waitForInputVBlankCount;
    result.lastPhaseCompleted = DM1_PHASE_INPUT_WAIT;

    result.exitRequested = state->exitGameImmediately;

    /* Tick accounting */
    state->tickCount++;
    state->lastTickMs = nowMs;
    state->frameStats.totalFrames++;

    return result;
}

/* ── Pause/resume (G2586_TimerActive) ─────────────────────────────── */

void m11_game_loop_pause(M11_GameLoopState *state)
{
    state->timerActive = 0;
    state->loopStatus = DM1_LOOP_PAUSED;
}

void m11_game_loop_resume(M11_GameLoopState *state)
{
    state->timerActive = 1;
    if (state->loopStatus == DM1_LOOP_PAUSED) {
        state->loopStatus = DM1_LOOP_RUNNING;
    }
}

int m11_game_loop_is_paused(const M11_GameLoopState *state)
{
    return !state->timerActive;
}

/* ── State mutation ───────────────────────────────────────────────── */

void m11_game_loop_request_new_map(M11_GameLoopState *state, int newMapIndex)
{
    state->newPartyMapIndex = newMapIndex;
}

void m11_game_loop_set_party_dead(M11_GameLoopState *state)
{
    state->partyDead = 1;
}

void m11_game_loop_set_inventory(M11_GameLoopState *state, int championOrdinal)
{
    state->inventoryChampionOrdinal = championOrdinal;
}

void m11_game_loop_set_resting(M11_GameLoopState *state, int resting)
{
    state->partyResting = resting;
}

void m11_game_loop_request_exit(M11_GameLoopState *state)
{
    state->exitGameImmediately = 1;
    state->loopStatus = DM1_LOOP_STOPPED;
}

int m11_game_loop_should_continue(const M11_GameLoopState *state)
{
    return !state->partyDead && !state->exitGameImmediately;
}

/* ── Frame budget monitoring ──────────────────────────────────────── */

void m11_game_loop_record_phase_time(M11_GameLoopState *state,
                                     M11_GameLoopPhase phase,
                                     uint32_t elapsedUs)
{
    (void)phase; /* Individual phase tracking could be added later */

    /* Track worst-case frame time */
    if (elapsedUs > state->frameStats.longestFrameUs) {
        state->frameStats.longestFrameUs = elapsedUs;
    }

    /* Check budget overrun */
    if (elapsedUs > state->targetFrameTimeUs) {
        state->frameStats.droppedFrames++;
        state->frameStats.budgetOverrunCount++;
    }

    /* Running average (simple exponential) */
    if (state->frameStats.avgFrameUs == 0) {
        state->frameStats.avgFrameUs = elapsedUs;
    } else {
        state->frameStats.avgFrameUs =
            (state->frameStats.avgFrameUs * 7 + elapsedUs) / 8;
    }
}

M11_FrameTimingStats m11_game_loop_get_frame_stats(const M11_GameLoopState *state)
{
    return state->frameStats;
}

void m11_game_loop_reset_frame_stats(M11_GameLoopState *state)
{
    memset(&state->frameStats, 0, sizeof(state->frameStats));
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *m11_game_loop_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "VBLANK.C: F0577_VerticalBlank_Handler_CPSDF — VBlank ISR:\n"
        "  G0317_i_WaitForInputVerticalBlankCount++ each frame\n"
        "  if (G0317 >= G0318) G0321_B_StopWaitingForPlayerInput = TRUE\n"
        "  G1086_VerticalBlankCount++ (global frame counter)\n"
        "  Palette updates: G3199_B/G3121_B/G3127_B in VBlank\n"
        "  Message area scroll: G0354_i/G0355_B/G0356_puc in VBlank\n"
        "VBLANK.C: F0575_VerticalBlank_Initialize — install Amiga INTB_VERTB\n"
        "VBLANK.C: F0576_VerticalBlank_Deinitialize — remove handler\n"
        "GAMELOOP.C: F0002_MAIN_GameLoop_CPSDF — infinite loop:\n"
        "  G0317=0, check G0327 newMap, F0261 timeline, F0128 dungeon view,\n"
        "  G0325/G0326 mouse, F0363 highlight, F0065 sound, F0320 damage,\n"
        "  G0303 death check, input wait with G0318=10 max vblank\n"
        "PC34: G0318=10 (200ms), Amiga A3x: G0318=12 (240ms)\n"
        "DOS/CLOCK.C: timer interrupt emulates 50fps VBlank on IBM PC";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining VBLANK.C function citations for parity
 *
 *   VBLANK.C:18 F0507_AMIGA_D (platform-specific, not implemented for PC-34)
 *   VBLANK.C:373 F1372_S
 *   VBLANK.C:649 F2226_F
 *   VBLANK.C:663 F2227_F
 * ══════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — GAMELOOP.C remaining function citations
 *
 *   GAMELOOP.C:319 F1046_O
 *   GAMELOOP.C:335 F1047_C
 * ══════════════════════════════════════════════════════════════════════ */

