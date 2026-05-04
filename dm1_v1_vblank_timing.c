#include "dm1_v1_vblank_timing.h"

#include <string.h>

/*
 * DM1 V1 VBlank timing implementation.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 * VBLANK.C:F0577_VerticalBlank_Handler_CPSDF — The VBlank interrupt
 * handler increments G0317_i_WaitForInputVerticalBlankCount each
 * vertical blank.  When it reaches G0318_i_WaitForInputMaximumVerticalBlankCount,
 * it sets G0321_B_StopWaitingForPlayerInput = C1_TRUE.
 *
 * GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF — At the top of each iteration:
 *   G0318 = 10 (MEDIA029: ST/Amiga/early PC) or 12 (MEDIA722: later media)
 *   G0317 = 0 (reset counter)
 *   Then the input wait loop:
 *     G0321 = FALSE
 *     do { keys; queue; } while (!G0321 || !G0301)
 *   After the loop: G0310--, G0311-- (movement cooldown decrement)
 *
 * CLIKMENU.C:142-179 F0365 — Turn handling: modifies party direction
 *   within the input poll loop.  Does NOT set movement cooldown.
 *
 * CLIKMENU.C:330-346 — After a successful step: sets G0310 to the
 *   maximum F0310_CHAMPION_GetMovementTicks among living champions,
 *   clears G0311.
 *
 * On PAL (50Hz), each VBlank = 20ms.  With maxVBlank = 10:
 *   Game tick = 10 * 20ms = 200ms = 5 ticks/second.
 *
 * The previous Firestaff approximation used 166ms (~6 ticks/sec), which
 * is 17% too fast.  This implementation corrects to authentic PAL timing.
 */

void DM1_V1_VBlankTiming_Init(DM1_V1_VBlankTimingState* state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->maxVBlankCount = DM1_V1_MAX_VBLANK_COUNT_ORIGINAL;
    state->gameTimeTicking = 1;
}

int DM1_V1_VBlankTiming_Update(DM1_V1_VBlankTimingState* state,
                                uint32_t elapsedMs)
{
    int tickReached = 0;

    if (!state) return 0;

    state->vblankAccumulatorMs += elapsedMs;

    /* Simulate VBlank interrupts at PAL rate.
     * Each VBlank = 20ms.  For each 20ms elapsed, increment the
     * VBlank counter just like F0577 does on the real hardware. */
    while (state->vblankAccumulatorMs >= DM1_V1_PAL_VBLANK_MS) {
        state->vblankAccumulatorMs -= DM1_V1_PAL_VBLANK_MS;
        state->vblankCount++;

        /* F0577: if (G0317 >= G0318) G0321 = TRUE */
        if (state->vblankCount >= state->maxVBlankCount) {
            if (!state->stopWaitingForInput) {
                state->stopWaitingForInput = 1;
                tickReached = 1;
            }
        }
    }

    /* Update turn cooldown tracking */
    if (state->turnCooldownMs > elapsedMs) {
        state->turnCooldownMs -= elapsedMs;
    } else {
        state->turnCooldownMs = 0;
    }

    return tickReached;
}

void DM1_V1_VBlankTiming_ResetForNewTick(DM1_V1_VBlankTimingState* state)
{
    if (!state) return;

    /* GAMELOOP.C top of loop: G0317 = 0 */
    state->vblankCount = 0;

    /* GAMELOOP.C input wait entry: G0321 = FALSE */
    state->stopWaitingForInput = 0;

    /* GAMELOOP.C:150-155 — Decrement movement cooldowns once per tick */
    if (state->disabledMovementTicks > 0) {
        state->disabledMovementTicks--;
    }
    if (state->projectileDisabledMovementTicks > 0) {
        state->projectileDisabledMovementTicks--;
    }
}

int DM1_V1_VBlankTiming_TurnAllowed(const DM1_V1_VBlankTimingState* state,
                                     uint32_t turnIntervalMs)
{
    if (!state) return 0;
    /* Turns execute within the input poll loop in the original.
     * They are gated by the tick boundary but not by movement cooldown.
     * We gate them by a minimum interval matching the tick rate. */
    return state->turnCooldownMs == 0 || turnIntervalMs == 0;
}

void DM1_V1_VBlankTiming_RecordTurn(DM1_V1_VBlankTimingState* state)
{
    if (!state) return;
    /* Set cooldown to one full tick interval.
     * In the original, the next turn can only happen after the VBlank
     * counter resets at the next tick boundary.  This is approximately
     * one tick interval. */
    state->turnCooldownMs = DM1_V1_GAME_TICK_INTERVAL_MS;
}

int DM1_V1_VBlankTiming_MovementAllowed(const DM1_V1_VBlankTimingState* state)
{
    if (!state) return 0;
    /* COMMAND.C:2095-2100 — Movement commands are held while G0310 > 0 */
    return state->disabledMovementTicks <= 0;
}

void DM1_V1_VBlankTiming_ApplyMovementCooldown(
    DM1_V1_VBlankTimingState* state,
    int cooldownTicks)
{
    if (!state) return;
    /* CLIKMENU.C:330-346 — After successful step, set G0310 to max
     * champion movement ticks.  G0311 is cleared (set to 0). */
    state->disabledMovementTicks = cooldownTicks;
    state->projectileDisabledMovementTicks = 0;
}

const char* DM1_V1_VBlankTiming_SourceEvidence(void)
{
    return "VBLANK.C:F0577 G0317++/G0321 gate; "
           "GAMELOOP.C:F0002 G0318=10(MEDIA029)/12(MEDIA722), "
           "G0317=0 reset, G0310--/G0311-- per tick; "
           "CLIKMENU.C:142-179 F0365 turn (no cooldown); "
           "CLIKMENU.C:330-346 step cooldown = max(F0310); "
           "PAL 50Hz: 10*20ms = 200ms/tick = 5 ticks/sec";
}
