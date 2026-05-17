/*
 * dm1_v1_movement_pc34_compat.c
 *
 * Firestaff DM1 V1 — Movement command dispatch + step/timing loop.
 * Implementation of the isolated reference movement system.
 *
 * ══════════════════════════════════════════════════════════════════════
 * ReDMCSB SOURCE LOCK — IMPLEMENTATION REFERENCES
 * ══════════════════════════════════════════════════════════════════════
 *
 * Every function below cites the exact ReDMCSB source file, function name,
 * and approximate line numbers for the logic it models.
 *
 * Source base: ReDMCSB_WIP20210206/Toolchains/Common/Source/
 * ══════════════════════════════════════════════════════════════════════
 */

#include "dm1_v1_movement_pc34_compat.h"
#include <string.h>  /* memset */

/* ── Direction delta tables ─────────────────────────────────────────
 * Ref: CLIKMENU.C ~line 218-226:
 *   G0465_ai_Graphic561_MovementArrowToStepForwardCount[4] = { 1, 0, -1, 0 }
 *   G0466_ai_Graphic561_MovementArrowToStepRightCount[4]   = { 0, 1,  0, -1 }
 *
 * Index = command - C003_COMMAND_MOVE_FORWARD:
 *   0=Forward, 1=Right, 2=Backward, 3=Left
 */
static const int16_t step_forward_count[4] = {  1,  0, -1,  0 };
static const int16_t step_right_count[4]   = {  0,  1,  0, -1 };

/* ── Cardinal direction offsets ─────────────────────────────────────
 * Ref: DUNGEON.C F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:
 *   Direction 0 (North): dy = -1   Direction 1 (East):  dx = +1
 *   Direction 2 (South): dy = +1   Direction 3 (West):  dx = -1
 */
static const int16_t dir_dx[4] = {  0,  1,  0, -1 };
static const int16_t dir_dy[4] = { -1,  0,  1,  0 };

/* ── Normalize direction to 0-3 ─────────────────────────────────────
 * Ref: DEFS.H M021_NORMALIZE macro: ((val) & 0x0003)
 */
static inline int16_t normalize_dir(int16_t d) {
    return d & 0x0003;
}


/*
 * dm1v1_movement_init
 *
 * Ref: GAMELOOP.C F0002_MAIN_GameLoop_CPSDF (~line 28-35):
 *   - G0318_i_WaitForInputMaximumVerticalBlankCount = 10
 *   - G0317_i_WaitForInputVerticalBlankCount = 0 (reset each loop iteration)
 *   - G0310_i_DisabledMovementTicks starts at 0 (implicit: movement is allowed)
 *   - G0311_i_ProjectileDisabledMovementTicks starts at 0
 *   - Party position is loaded from dungeon data or save game
 */
void dm1v1_movement_init(DM1_V1_MovementState *state,
                         int16_t start_x, int16_t start_y,
                         int16_t start_facing)
{
    memset(state, 0, sizeof(*state));
    state->pos_x  = start_x;
    state->pos_y  = start_y;
    state->facing  = normalize_dir(start_facing);
    state->phase   = DM1_V1_PHASE_IDLE;
    state->step_timer = 0;               /* G0310_i_DisabledMovementTicks = 0 */
    state->projectile_lockout_timer = 0;  /* G0311_i_ProjectileDisabledMovementTicks = 0 */
    state->projectile_lockout_dir = 0;
    state->step_cost = 2;               /* default unencumbered cost */
}


/*
 * dm1v1_command_queue_init
 *
 * Ref: COMMAND.C globals (~line 10-12):
 *   G0433_i_CommandQueueFirstIndex = 0
 *   G0434_i_CommandQueueLastIndex = M529_COMMAND_QUEUE_SIZE (= 4)
 *   G0435_B_CommandQueueLocked = C1_TRUE
 *
 * The queue is empty when (last + 1) % (QUEUE_SIZE+1) == first.
 * Initial state: last=4, first=0 → (4+1)%5 = 0 == 0 → empty. ✓
 */
void dm1v1_command_queue_init(DM1_V1_CommandQueue *queue)
{
    memset(queue, 0, sizeof(*queue));
    queue->first_index = 0;                          /* G0433 = 0 */
    queue->last_index  = DM1_V1_COMMAND_QUEUE_CAPACITY;  /* G0434 = M529_COMMAND_QUEUE_SIZE */
    queue->locked      = 1;                          /* G0435 = C1_TRUE */
}


/*
 * dm1v1_movement_poll_input — Enqueue a single command code.
 *
 * Ref: COMMAND.C F0359_COMMAND_ProcessClick_CPSC (~line 1470-1590):
 *   1. Check G0435_B_CommandQueueLocked; if locked, set pending click and return
 *   2. Compute next index: (G0434_i_CommandQueueLastIndex + 2) > M529_COMMAND_QUEUE_SIZE
 *      → subtract (M529_COMMAND_QUEUE_SIZE + 1)
 *   3. If next == G0433_i_CommandQueueFirstIndex → queue full, return
 *   4. Look up command via F0358_COMMAND_GetCommandFromMouseInput_CPSC
 *   5. If command != C000_COMMAND_NONE:
 *      - Decrement the write index: if (!L1108--) L1108 = M529_COMMAND_QUEUE_SIZE
 *      - Store command at G0432_as_CommandQueue[G0434 = L1108].Command
 *   6. Unlock: G0435_B_CommandQueueLocked = C0_FALSE
 *
 * Also models F0361_COMMAND_ProcessKeyPress (~line 1632) which uses
 * identical queue insertion arithmetic.
 */
int dm1v1_movement_poll_input(DM1_V1_CommandQueue *queue,
                              int16_t command_code)
{
    int16_t next_index;

    /* Ref: COMMAND.C ~line 1492: if queue is locked, buffer as pending click.
     * We model the unlocked path (normal operation after init). */
    if (queue->locked) {
        queue->locked = 0;  /* G0435 = C0_FALSE; auto-unlock for first use */
    }

    /* Ref: F0359 ~line 1498-1500:
     * if ((L1108 = G0434 + 2) > M529_COMMAND_QUEUE_SIZE)
     *     L1108 -= M529_COMMAND_QUEUE_SIZE + 1;
     */
    next_index = queue->last_index + 2;
    if (next_index > DM1_V1_COMMAND_QUEUE_CAPACITY) {
        next_index -= (DM1_V1_COMMAND_QUEUE_CAPACITY + 1);
    }

    /* Ref: F0359 ~line 1502:
     * if (L1108 == G0433) { return; } // queue full */
    if (next_index == queue->first_index) {
        return 0;  /* Queue full — command dropped */
    }

    /* Ref: F0361 ~line 1667-1672 (key) or F0359 ~line 1574-1580 (mouse):
     * if (!L1108--) L1108 = M529_COMMAND_QUEUE_SIZE;
     * G0432[G0434 = L1108].Command = command;
     */
    if (command_code == 0) {
        return 0;  /* C000_COMMAND_NONE — nothing to enqueue */
    }

    if (!(next_index--)) {
        next_index = DM1_V1_COMMAND_QUEUE_CAPACITY;
    }
    queue->commands[next_index] = command_code;
    queue->last_index = next_index;

    return 1;  /* Successfully enqueued */
}


/*
 * dm1v1_movement_execute_step — Dequeue and execute one command.
 *
 * Ref: COMMAND.C F0380_COMMAND_ProcessQueue_CPSC (~line 2045-2200):
 *
 * Line ~2061: G0435_B_CommandQueueLocked = C1_TRUE;
 * Line ~2063: if ((AL1159 = G0434 + 1) > M529_COMMAND_QUEUE_SIZE) AL1159 = 0;
 * Line ~2069: if (AL1159 == G0433) { goto T0380xxx; } // queue empty
 * Line ~2071: L1160 = G0432[G0433].Command;
 * Line ~2073: if ((L1160 >= C003) && (L1160 <= C006) &&
 *                 (G0310_i_DisabledMovementTicks ||
 *                  (G0311 && G0312 == NORMALIZE(G0308 + L1160 - C003))))
 *             { goto T0380xxx; } // movement locked
 * Line ~2086: L1161 = G0432[G0433].X;  L1162 = G0432[G0433].Y;
 * Line ~2089: if (++G0433 > M529_COMMAND_QUEUE_SIZE) G0433 = 0;
 * Line ~2091: G0435 = C0_FALSE;
 *
 * Line ~2104: if (L1160 == C002 || L1160 == C001)
 *                 F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160);
 * Line ~2108: if (L1160 >= C003 && L1160 <= C006)
 *                 F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160);
 *
 * --- Turn processing (CLIKMENU.C F0365 ~line 142-173):
 * Line ~158: G0321 = C1_TRUE;
 * Line ~172: F0284_CHAMPION_SetPartyDirection(NORMALIZE(G0308 + (cmd==RIGHT ? 1 : 3)));
 *
 * --- Move processing (CLIKMENU.C F0366 ~line 180-346):
 * Line ~224: AL1118 = P0735 - C003;
 * Line ~230-234: L1123 = (square_type == STAIRS); handle stairs specially
 * Line ~236: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(...)
 * Line ~244-296: Wall/door/fakewall/group blocking checks
 * Line ~318: F0267_MOVE_GetMoveResult_CPSCE(THING_PARTY, ...)
 * Line ~319-340: AL1115 = max(1, max(champion_ticks))
 * Line ~345: G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;
 * Line ~346: G0311_i_ProjectileDisabledMovementTicks = 0;
 */
int16_t dm1v1_movement_execute_step(DM1_V1_MovementState *state,
                                     DM1_V1_CommandQueue *queue)
{
    int16_t check_index;
    int16_t command;
    int16_t arrow_index;
    int16_t move_dir;
    int16_t new_x, new_y;
    int16_t fwd_count, right_count;
    int16_t abs_facing;

    /* Ref: F0380 ~line 2061: G0435 = C1_TRUE */
    queue->locked = 1;

    /* Ref: F0380 ~line 2063-2064:
     * if ((AL1159 = G0434 + 1) > M529_COMMAND_QUEUE_SIZE) AL1159 = 0; */
    check_index = queue->last_index + 1;
    if (check_index > DM1_V1_COMMAND_QUEUE_CAPACITY) {
        check_index = 0;
    }

    /* Ref: F0380 ~line 2069: queue empty check */
    if (check_index == queue->first_index) {
        /* Queue empty — no command to process */
        queue->locked = 0;  /* F0380 ~line 2079: G0435 = C0_FALSE */
        return 0;  /* C000_COMMAND_NONE */
    }

    /* Ref: F0380 ~line 2071: L1160 = G0432[G0433].Command */
    command = queue->commands[queue->first_index];

    /* Ref: F0380 ~line 2073-2078: Movement lockout gate.
     * if ((L1160 >= C003) && (L1160 <= C006) &&
     *     (G0310 || (G0311 && G0312 == NORMALIZE(G0308 + L1160 - C003))))
     *     → reject: don't dequeue, return NONE */
    if (command >= 3 && command <= 6) {  /* C003..C006 = movement commands */
        if (state->step_timer > 0) {
            /* G0310_i_DisabledMovementTicks != 0 → movement disabled */
            queue->locked = 0;
            return 0;
        }
        /* Ref: Projectile lockout check (F0380 ~line 2075-2077) */
        if (state->projectile_lockout_timer > 0) {
            move_dir = normalize_dir(state->facing + command - 3);
            if (move_dir == state->projectile_lockout_dir) {
                queue->locked = 0;
                return 0;
            }
        }
    }

    /* Ref: F0380 ~line 2089: dequeue — advance first_index */
    if (++queue->first_index > DM1_V1_COMMAND_QUEUE_CAPACITY) {
        queue->first_index = 0;
    }

    /* Ref: F0380 ~line 2091: unlock */
    queue->locked = 0;

    /* ── Dispatch: turns ── */
    /* Ref: F0380 ~line 2104-2106 → F0365 (CLIKMENU.C ~line 142) */
    if (command == 2 || command == 1) {
        /* C001_COMMAND_TURN_LEFT or C002_COMMAND_TURN_RIGHT */
        /* Ref: F0365 ~line 172:
         * F0284_CHAMPION_SetPartyDirection(NORMALIZE(G0308 + (cmd==RIGHT ? 1 : 3))) */
        if (command == 2) {  /* TURN_RIGHT */
            state->facing = normalize_dir(state->facing + 1);
        } else {  /* TURN_LEFT */
            state->facing = normalize_dir(state->facing + 3);
        }
        /* Ref: F0365 ~line 158: G0321_B_StopWaitingForPlayerInput = C1_TRUE */
        /* (No step_timer set for turns — they are instantaneous) */
        return command;
    }

    /* ── Dispatch: movement ── */
    /* Ref: F0380 ~line 2108-2110 → F0366 (CLIKMENU.C ~line 180) */
    if (command >= 3 && command <= 6) {
        /* Ref: F0366 ~line 224: AL1118 = P0735 - C003 */
        arrow_index = command - 3;

        /* Ref: F0366 ~line 236 → F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:
         * Compute new position using party facing + movement arrow deltas.
         *
         * The original uses F0150 which takes (direction, forward_count, right_count, &x, &y).
         * Forward: along the facing direction.
         * Right:   perpendicular clockwise from facing. */
        fwd_count   = step_forward_count[arrow_index];
        right_count = step_right_count[arrow_index];

        abs_facing = state->facing;
        new_x = state->pos_x
                + fwd_count   * dir_dx[abs_facing]
                + right_count * dir_dx[normalize_dir(abs_facing + 1)];
        new_y = state->pos_y
                + fwd_count   * dir_dy[abs_facing]
                + right_count * dir_dy[normalize_dir(abs_facing + 1)];

        /* NOTE: In the full engine, CLIKMENU.C F0366 ~line 244-296 performs
         * wall/door/fakewall/group blocking checks and may reject the move.
         * This reference implementation does NOT model blocking — it assumes
         * the move is valid. The existing Firestaff movement pipeline
         * (dm1_v1_movement_pipeline_pc34_compat.c) handles collision. */

        /* Ref: F0366 ~line 318:
         * F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, old_x, old_y, new_x, new_y)
         * → Updates G0306_i_PartyMapX, G0307_i_PartyMapY */
        state->pos_x = new_x;
        state->pos_y = new_y;

        /* Ref: F0366 ~line 319-340: Compute step duration.
         * AL1115_ui_Ticks = 1;
         * for each champion: if alive, AL1115 = MAX(AL1115, F0310_GetMovementTicks(champ))
         *
         * F0310_CHAMPION_GetMovementTicks returns ticks based on champion load/stats.
         * Default unencumbered: ~2 ticks. We use a constant approximation here. */
        state->step_timer = (state->step_cost > 0) ? state->step_cost : 2;

        /* Ref: F0366 ~line 345-346:
         * G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;
         * G0311_i_ProjectileDisabledMovementTicks = 0; */
        state->projectile_lockout_timer = 0;

        /* Ref: F0366 ~line 213: G0321_B_StopWaitingForPlayerInput = C1_TRUE */
        state->phase = DM1_V1_PHASE_STEPPING;

        return command;
    }

    /* Command was recognized but not a movement/turn — return it anyway */
    return command;
}


/*
 * dm1v1_movement_process_vertical_blank — VBlank tick handler.
 *
 * Ref: VBLANK.C F0577_VerticalBlank_Handler_CPSDF (~line 38-43):
 *   G0317_i_WaitForInputVerticalBlankCount++;
 *   if (G0317 >= G0318_i_WaitForInputMaximumVerticalBlankCount)
 *       G0321_B_StopWaitingForPlayerInput = C1_TRUE;
 *
 * Ref: GAMELOOP.C F0002_MAIN_GameLoop_CPSDF (~line 131-134):
 *   if (G0310_i_DisabledMovementTicks) {
 *       G0310_i_DisabledMovementTicks--;
 *   }
 *   if (G0311_i_ProjectileDisabledMovementTicks) {
 *       G0311_i_ProjectileDisabledMovementTicks--;
 *   }
 *
 * Phase transitions:
 *   STEPPING + timer > 0 → timer--, stay STEPPING
 *   STEPPING + timer == 0 → WAITING (step complete, awaiting cleanup)
 *   WAITING → IDLE (ready for next command)
 *   IDLE → IDLE (no-op)
 */
void dm1v1_movement_process_vertical_blank(DM1_V1_MovementState *state)
{
    /* Ref: GAMELOOP.C ~line 131-132:
     * if (G0310_i_DisabledMovementTicks) G0310_i_DisabledMovementTicks--; */
    if (state->step_timer > 0) {
        state->step_timer--;
    }

    /* Ref: GAMELOOP.C ~line 133-134:
     * if (G0311_i_ProjectileDisabledMovementTicks) ...--; */
    if (state->projectile_lockout_timer > 0) {
        state->projectile_lockout_timer--;
    }

    /* Phase state machine */
    switch (state->phase) {
        case DM1_V1_PHASE_STEPPING:
            if (state->step_timer == 0) {
                state->phase = DM1_V1_PHASE_WAITING;
            }
            break;
        case DM1_V1_PHASE_WAITING:
            state->phase = DM1_V1_PHASE_IDLE;
            break;
        case DM1_V1_PHASE_IDLE:
        default:
            break;
    }
}


/*
 * dm1v1_movement_is_in_movement — Query whether currently mid-step.
 *
 * Ref: F0380_COMMAND_ProcessQueue_CPSC (~line 2073-2078):
 *   Movement commands C003-C006 are rejected when:
 *     G0310_i_DisabledMovementTicks != 0
 *   So "in movement" = DisabledMovementTicks > 0 OR phase is not IDLE.
 */
int dm1v1_movement_is_in_movement(const DM1_V1_MovementState *state)
{
    return (state->phase != DM1_V1_PHASE_IDLE) || (state->step_timer > 0);
}


/*
 * dm1v1_movement_set_step_cost — Set configurable step duration.
 *
 * Callers should compute this per ReDMCSB CHAMPION.C F0310:
 *   cost = max(1, max(F0310_GetMovementTicks(champ) for each alive champion))
 * F0310 factors in champion load, stamina, speed boots.
 * Default is 2 (typical unencumbered movement).
 */
void dm1v1_movement_set_step_cost(DM1_V1_MovementState *state, int16_t cost)
{
    if (!state) return;
    state->step_cost = (cost > 0) ? cost : 2;
}
