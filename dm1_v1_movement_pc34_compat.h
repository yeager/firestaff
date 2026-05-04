/*
 * dm1_v1_movement_pc34_compat.h
 *
 * Firestaff DM1 V1 — Movement command dispatch + step/timing loop.
 * PC-34 compatibility layer for party movement state, command FIFO queue,
 * and vertical-blank-driven step timing.
 *
 * ══════════════════════════════════════════════════════════════════════
 * ReDMCSB SOURCE LOCK — MANDATORY REVIEW RECORD
 * ══════════════════════════════════════════════════════════════════════
 *
 * Source path: ReDMCSB_WIP20210206/Toolchains/Common/Source/
 *
 * COMMAND.C — Command queue globals and dispatch:
 *   G0432_as_CommandQueue[]           (line ~5)   — Circular queue array
 *   G0433_i_CommandQueueFirstIndex    (line ~10)  — Queue head
 *   G0434_i_CommandQueueLastIndex     (line ~11)  — Queue tail
 *   G0435_B_CommandQueueLocked        (line ~12)  — Atomic lock flag
 *   M529_COMMAND_QUEUE_SIZE           (DEFS.H)    — Queue capacity (4+1)
 *   F0361_COMMAND_ProcessKeyPress     (line ~1632)— Key-to-command dispatch
 *   F0359_COMMAND_ProcessClick_CPSC   (line ~1470)— Mouse-to-command dispatch
 *   F0380_COMMAND_ProcessQueue_CPSC   (line ~2045)— Dequeue + dispatch loop
 *
 * CLIKMENU.C — Movement command processing:
 *   F0365_COMMAND_ProcessTypes1To2_TurnParty       (line ~142) — Turn left/right
 *   F0366_COMMAND_ProcessTypes3To6_MoveParty       (line ~180) — Move fwd/back/left/right
 *   G0465_ai_..._MovementArrowToStepForwardCount   (line ~218) — Direction deltas
 *   G0466_ai_..._MovementArrowToStepRightCount     (line ~222) — Direction deltas
 *   G0310_i_DisabledMovementTicks                  (line ~345) — Step lockout timer
 *   G0311_i_ProjectileDisabledMovementTicks         (line ~346) — Projectile lockout
 *   F0310_CHAMPION_GetMovementTicks                (CHAMPION.C) — Per-champion tick cost
 *
 * VBLANK.C — Vertical blank timing:
 *   F0577_VerticalBlank_Handler_CPSDF  (line ~38) — VBlank ISR
 *   G0317_i_WaitForInputVerticalBlankCount (line ~39) — VBlank frame counter
 *   G0318_i_WaitForInputMaximumVerticalBlankCount  — Max wait (10 or 12)
 *   G0321_B_StopWaitingForPlayerInput   (line ~42) — Input-wait break flag
 *
 * GAMELOOP.C — Main game loop:
 *   F0002_MAIN_GameLoop_CPSDF          (line ~28) — Outer loop + VBlank reset
 *   G0310_i_DisabledMovementTicks--    (line ~131)— Decrement per tick
 *   G0311_i_ProjectileDisabledMovementTicks-- (line ~134) — Decrement per tick
 *
 * MOVESENS.C — Movement/sensor processing:
 *   G0397_i_MoveResultMapX             (line ~4)  — Result X after move
 *   G0398_i_MoveResultMapY             (line ~5)  — Result Y after move
 *   G0400_i_MoveResultDirection        (line ~7)  — Result facing after move
 *   F0267_MOVE_GetMoveResult_CPSCE     — Core move-result processor
 *
 * DEFS.H — Command constants:
 *   C001_COMMAND_TURN_LEFT     = 1
 *   C002_COMMAND_TURN_RIGHT    = 2
 *   C003_COMMAND_MOVE_FORWARD  = 3
 *   C004_COMMAND_MOVE_RIGHT    = 4
 *   C005_COMMAND_MOVE_BACKWARD = 5
 *   C006_COMMAND_MOVE_LEFT     = 6
 *   C000_COMMAND_NONE          = 0
 * ══════════════════════════════════════════════════════════════════════
 */

#ifndef DM1_V1_MOVEMENT_PC34_COMPAT_H
#define DM1_V1_MOVEMENT_PC34_COMPAT_H

#include <stdint.h>

/* ── Direction enum ─────────────────────────────────────────────────
 * Maps to ReDMCSB cardinal directions (0=North, 1=East, 2=South, 3=West).
 * DM1_V1_DIR_STAY represents "no movement" / turn-only / idle.
 * Ref: DEFS.H direction constants, M021_NORMALIZE macro.
 */
typedef enum {
    DM1_V1_DIR_NORTH = 0,
    DM1_V1_DIR_EAST  = 1,
    DM1_V1_DIR_SOUTH = 2,
    DM1_V1_DIR_WEST  = 3,
    DM1_V1_DIR_STAY  = 4  /* sentinel: no directional movement */
} DM1_V1_Direction;

/* ── Movement phase ─────────────────────────────────────────────────
 * Models the three-phase cycle from ReDMCSB:
 *   IDLE    → No movement pending
 *             (F0380: queue empty → T0380042, GAMELOOP.C line ~215)
 *   STEPPING → Actively executing a step; DisabledMovementTicks > 0
 *             (CLIKMENU.C F0366 line ~345: G0310_i_DisabledMovementTicks = AL1115_ui_Ticks)
 *   WAITING → Step complete, waiting for VBlank to advance to next tick
 *             (GAMELOOP.C line ~131: G0310_i_DisabledMovementTicks--)
 */
typedef enum {
    DM1_V1_PHASE_IDLE     = 0,
    DM1_V1_PHASE_STEPPING = 1,
    DM1_V1_PHASE_WAITING  = 2
} DM1_V1_MovementPhase;

/* ── Movement state ─────────────────────────────────────────────────
 * Consolidates the scattered globals from GAMELOOP.C, MOVESENS.C, and CLIKMENU.C
 * into a single trackable struct.
 *
 * Ref: G0306_i_PartyMapX, G0307_i_PartyMapY (party position)
 *      G0308_i_PartyDirection (facing)
 *      G0310_i_DisabledMovementTicks (step timer)
 *      G0311_i_ProjectileDisabledMovementTicks (projectile lockout)
 */
typedef struct {
    int16_t  pos_x;                    /* G0306_i_PartyMapX */
    int16_t  pos_y;                    /* G0307_i_PartyMapY */
    int16_t  facing;                   /* G0308_i_PartyDirection (0-3) */
    DM1_V1_MovementPhase phase;        /* current movement phase */
    int16_t  step_timer;               /* G0310_i_DisabledMovementTicks */
    int16_t  projectile_lockout_timer; /* G0311_i_ProjectileDisabledMovementTicks */
    int16_t  projectile_lockout_dir;   /* G0312_i_LastProjectileDisabledMovementDirection */
} DM1_V1_MovementState;

/* ── Command queue ──────────────────────────────────────────────────
 * Models the original circular FIFO from COMMAND.C:
 *   G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1] — 5-slot circular buffer
 *   G0433_i_CommandQueueFirstIndex — read head
 *   G0434_i_CommandQueueLastIndex  — write tail (empty when (tail+1)%5 == head)
 *   G0435_B_CommandQueueLocked     — atomic guard flag
 *
 * Queue capacity: M529_COMMAND_QUEUE_SIZE = 4 actual commands.
 * The +1 slot distinguishes full from empty.
 */
#define DM1_V1_COMMAND_QUEUE_CAPACITY 4  /* M529_COMMAND_QUEUE_SIZE */
#define DM1_V1_COMMAND_QUEUE_SLOTS    (DM1_V1_COMMAND_QUEUE_CAPACITY + 1)

typedef struct {
    int16_t  commands[DM1_V1_COMMAND_QUEUE_SLOTS]; /* command codes */
    int16_t  first_index;    /* G0433_i_CommandQueueFirstIndex */
    int16_t  last_index;     /* G0434_i_CommandQueueLastIndex */
    int16_t  locked;         /* G0435_B_CommandQueueLocked */
} DM1_V1_CommandQueue;

/* ── Function declarations ──────────────────────────────────────────
 *
 * All functions model specific ReDMCSB code paths as documented in
 * the source-lock record above. They do NOT replace the existing
 * Firestaff movement pipeline; they provide an isolated reference
 * implementation for verification and testing.
 */

/*
 * dm1v1_movement_init — Initialize movement state from starting position.
 *
 * Ref: GAMELOOP.C F0002_MAIN_GameLoop_CPSDF initialization block (~line 28-35):
 *      G0317_i_WaitForInputVerticalBlankCount = 0
 *      G0318_i_WaitForInputMaximumVerticalBlankCount = 10 (Atari ST/Amiga/PC)
 *      G0310_i_DisabledMovementTicks starts at 0 (movement allowed)
 */
void dm1v1_movement_init(DM1_V1_MovementState *state,
                         int16_t start_x, int16_t start_y,
                         int16_t start_facing);

/*
 * dm1v1_command_queue_init — Initialize command queue to empty state.
 *
 * Ref: COMMAND.C globals (~line 10-12):
 *      G0433_i_CommandQueueFirstIndex = 0
 *      G0434_i_CommandQueueLastIndex = M529_COMMAND_QUEUE_SIZE (=4)
 *      G0435_B_CommandQueueLocked = C1_TRUE (initially locked)
 */
void dm1v1_command_queue_init(DM1_V1_CommandQueue *queue);

/*
 * dm1v1_movement_poll_input — Read input, enqueue command if valid.
 *
 * Models F0361_COMMAND_ProcessKeyPress (COMMAND.C ~line 1632) and
 * F0359_COMMAND_ProcessClick_CPSC (COMMAND.C ~line 1470).
 *
 * Enqueues a command into the circular queue using the same index
 * arithmetic as the original: advance tail, store, check capacity.
 *
 * Returns 1 if a command was successfully enqueued, 0 otherwise.
 */
int dm1v1_movement_poll_input(DM1_V1_CommandQueue *queue,
                              int16_t command_code);

/*
 * dm1v1_movement_execute_step — Execute one movement step from command queue.
 *
 * Models F0380_COMMAND_ProcessQueue_CPSC (COMMAND.C ~line 2045):
 *   1. Check queue non-empty: (last+1) % 5 != first
 *   2. If command is MOVE (3-6), check G0310_i_DisabledMovementTicks == 0
 *   3. Dequeue from first_index, advance head
 *   4. Dispatch to F0365 (turn) or F0366 (move)
 *   5. For moves: update position, set step_timer = max(champion ticks)
 *
 * Returns the command code that was executed, or 0 (C000_COMMAND_NONE).
 */
int16_t dm1v1_movement_execute_step(DM1_V1_MovementState *state,
                                     DM1_V1_CommandQueue *queue);

/*
 * dm1v1_movement_process_vertical_blank — Called once per VBlank tick.
 *
 * Models the VBlank handler + game loop tick decrement:
 *   VBLANK.C F0577 (~line 38): G0317_i_WaitForInputVerticalBlankCount++
 *   GAMELOOP.C (~line 131): if (G0310_i_DisabledMovementTicks) G0310_i_DisabledMovementTicks--
 *   GAMELOOP.C (~line 134): if (G0311_i_ProjectileDisabledMovementTicks) ...--
 *
 * Advances step_timer toward zero. Transitions phase from STEPPING→WAITING
 * when timer reaches 0, and from WAITING→IDLE on the next call.
 */
void dm1v1_movement_process_vertical_blank(DM1_V1_MovementState *state);

/*
 * dm1v1_movement_is_in_movement — Query whether currently mid-step.
 *
 * Returns 1 if phase != IDLE (i.e., G0310_i_DisabledMovementTicks > 0
 * or waiting for final VBlank completion), 0 otherwise.
 *
 * Ref: F0380_COMMAND_ProcessQueue_CPSC (~line 2076):
 *      Movement commands are rejected when G0310_i_DisabledMovementTicks != 0
 */
int dm1v1_movement_is_in_movement(const DM1_V1_MovementState *state);

#endif /* DM1_V1_MOVEMENT_PC34_COMPAT_H */
