#ifndef DM1_V1_VBLANK_TIMING_H
#define DM1_V1_VBLANK_TIMING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 VBlank-based timing system.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 * - VBLANK.C:F0577 increments G0317_i_WaitForInputVerticalBlankCount
 *   every VBlank interrupt (50 Hz on PAL Amiga/Atari ST).  When the
 *   counter reaches G0318_i_WaitForInputMaximumVerticalBlankCount,
 *   it sets G0321_B_StopWaitingForPlayerInput = TRUE.
 *
 * - GAMELOOP.C:F0002 resets G0317 to 0 at the top of each game loop
 *   iteration, sets G0318 = 10 (ST/Amiga/early PC) or 12 (later media),
 *   then enters the input polling do-while:
 *     do { process_keys(); process_queue(); } while (!G0321 || !G0301);
 *   This means each game tick takes exactly G0318 VBlanks = 10/50Hz =
 *   200ms on PAL systems (5 ticks/second).
 *
 * - GAMELOOP.C:150-155 decrements G0310_i_DisabledMovementTicks and
 *   G0311_i_ProjectileDisabledMovementTicks once per game tick.
 *
 * For Firestaff, we simulate the VBlank counter using wall-clock
 * milliseconds.  The game tick interval is PAL_VBLANK_MS * maxVBlankCount
 * = 20ms * 10 = 200ms.
 */

/* -- PAL VBlank constants from ReDMCSB -- */

/* PAL vertical blank frequency: 50 Hz */
#define DM1_V1_PAL_VBLANK_HZ                50

/* Milliseconds per VBlank at PAL rate: 1000/50 = 20ms */
#define DM1_V1_PAL_VBLANK_MS                20

/* G0318_i_WaitForInputMaximumVerticalBlankCount for DM1 V1 (ST/Amiga/PC).
 * Ref: GAMELOOP.C MEDIA029 branch sets this to 10.
 * Later media (MEDIA722) uses 12. */
#define DM1_V1_MAX_VBLANK_COUNT_ORIGINAL    10
#define DM1_V1_MAX_VBLANK_COUNT_LATER       12

/* Authentic game tick interval in milliseconds.
 * PAL: 10 VBlanks x 20ms = 200ms = 5 ticks/second.
 * This replaces the previous 166ms approximation. */
#define DM1_V1_GAME_TICK_INTERVAL_MS        (DM1_V1_PAL_VBLANK_MS * DM1_V1_MAX_VBLANK_COUNT_ORIGINAL)

/* -- VBlank tick state -- */

typedef struct {
    /* Simulated G0317_i_WaitForInputVerticalBlankCount.
     * Counts VBlanks elapsed since last game tick reset. */
    int16_t vblankCount;

    /* Simulated G0318_i_WaitForInputMaximumVerticalBlankCount.
     * Number of VBlanks that must elapse before the game loop advances.
     * Default: DM1_V1_MAX_VBLANK_COUNT_ORIGINAL (10). */
    int16_t maxVBlankCount;

    /* Simulated G0321_B_StopWaitingForPlayerInput.
     * Set to 1 when vblankCount >= maxVBlankCount. */
    int stopWaitingForInput;

    /* Simulated G0301_B_GameTimeTicking.
     * Must be 1 for the game loop to advance past input wait.
     * Always 1 during normal gameplay; 0 during entrance/loading. */
    int gameTimeTicking;

    /* Wall-clock accumulator for VBlank simulation (milliseconds).
     * Tracks sub-VBlank time between updates. */
    uint32_t vblankAccumulatorMs;

    /* Movement cooldown gates (mirrors of G0310/G0311).
     * Decremented once per game tick, not per VBlank.
     * While > 0, movement commands are held in queue.
     * Ref: GAMELOOP.C:150-155 */
    int disabledMovementTicks;
    int projectileDisabledMovementTicks;

    /* Turn animation timing.
     * In the original, turns execute immediately within the input
     * poll loop (F0365 in CLIKMENU.C:142-179) and do NOT set
     * movement cooldown -- but the VBlank counter still gates the
     * next game tick.  Turns are "free" within a tick's input window
     * but cannot bypass the VBlank-gated tick boundary.
     *
     * turnCooldownMs tracks time since last turn to provide smooth
     * key-repeat-like behavior matching the original's feel. */
    uint32_t turnCooldownMs;
} DM1_V1_VBlankTimingState;

/* Initialize timing state with authentic PAL defaults. */
void DM1_V1_VBlankTiming_Init(DM1_V1_VBlankTimingState* state);

/* Simulate VBlank interrupts for the given elapsed milliseconds.
 * Call this every frame with real wall-clock delta.
 * Returns 1 if a game tick boundary was reached (stopWaitingForInput
 * transitioned to 1). */
int DM1_V1_VBlankTiming_Update(DM1_V1_VBlankTimingState* state,
                                uint32_t elapsedMs);

/* Reset VBlank counter for a new game tick.
 * Called at the top of each game loop iteration.
 * Mirrors GAMELOOP.C: G0317 = 0; G0321 = FALSE;
 * Also decrements movement cooldowns (G0310/G0311). */
void DM1_V1_VBlankTiming_ResetForNewTick(DM1_V1_VBlankTimingState* state);

/* Check if a turn is allowed (cooldown expired).
 * Turns in DM1 don't consume a movement cooldown tick but are still
 * gated by the VBlank-driven tick boundary.
 * turnIntervalMs: minimum ms between consecutive turns (default ~200ms). */
int DM1_V1_VBlankTiming_TurnAllowed(const DM1_V1_VBlankTimingState* state,
                                     uint32_t turnIntervalMs);

/* Record that a turn was executed, resetting the turn cooldown. */
void DM1_V1_VBlankTiming_RecordTurn(DM1_V1_VBlankTimingState* state);

/* Check if movement is allowed (cooldown expired).
 * Mirrors COMMAND.C:2095-2100 check on G0310. */
int DM1_V1_VBlankTiming_MovementAllowed(const DM1_V1_VBlankTimingState* state);

/* Apply movement cooldown after a successful step.
 * Sets disabledMovementTicks to the given tick count.
 * Mirrors CLIKMENU.C:330-346. */
void DM1_V1_VBlankTiming_ApplyMovementCooldown(
    DM1_V1_VBlankTimingState* state,
    int cooldownTicks);

/* Source evidence string for documentation. */
const char* DM1_V1_VBlankTiming_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_VBLANK_TIMING_H */
