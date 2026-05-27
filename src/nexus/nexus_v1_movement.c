#include "nexus_v1_movement.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════
 * DM1-compatible party movement system for Nexus V1.
 * Source: DM1 CLIKMENU.C F0366 COMMAND_ProcessTypes3To6_MoveParty
 *         CLIKMENU.C:264-276 stairs special cases
 *         CLIKMENU.C:237-255 living-champion stamina decrement
 *         CLIKMENU.C:224-233 arrow deltas / blocked movement
 *         MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE:316-328, 752-783
 *         CHAMPION.C F0325_CHAMPION_DecrementStamina:2025-2048
 * ═══════════════════════════════════════════════════════════════════ */

/* Direction delta lookup */
static const int g_dir_dx[4] = {0, 1, 0, -1};
static const int g_dir_dy[4] = {-1, 0, 1, 0};

/* Arrow deltas relative to party direction
 * Arrow 0=forward: move in facing dir
 * Arrow 1=backward: move in opposite dir
 * Arrow 2=turn left: no step
 * Arrow 3=turn right: no step
 * Arrow 4=strafe left: perpendicular
 * Arrow 5=strafe right: perpendicular */
static const int g_arrow_forward_delta[4] = {0, -1, 0, 1};  /* dx by facing */
static const int g_arrow_right_delta[4]  = {1, 0, -1, 0};  /* dx by facing */

/* ═══════════════════════════════════════════════════════════════════
 * Input queue
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_input_queue_init(Nexus_InputQueue *q) {
    if (!q) return;
    memset(q, 0, sizeof(*q));
}

int nexus_input_queue_push(Nexus_InputQueue *q, int command) {
    if (!q || command <= 0) return -1;
    if (q->count >= NEXUS_INPUT_QUEUE_SIZE) return -1;
    q->commands[q->tail].command = command;
    q->commands[q->tail].dequeued = 0;
    q->tail = (q->tail + 1) % NEXUS_INPUT_QUEUE_SIZE;
    q->count++;
    return 0;
}

int nexus_input_queue_pop(Nexus_InputQueue *q, int *out_cmd) {
    if (!q || q->count <= 0) return 0;
    if (out_cmd) *out_cmd = q->commands[q->head].command;
    q->commands[q->head].dequeued = 1;
    q->head = (q->head + 1) % NEXUS_INPUT_QUEUE_SIZE;
    q->count--;
    return 1;
}

int nexus_input_queue_count(const Nexus_InputQueue *q) {
    return q ? q->count : 0;
}

void nexus_input_queue_discard(Nexus_InputQueue *q) {
    if (!q) return;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

/* ═══════════════════════════════════════════════════════════════════
 * Direction helpers
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_dir_deltas(int dir, int *dx, int *dy) {
    int d = ((dir % 4) + 4) % 4;
    if (dx) *dx = g_dir_dx[d];
    if (dy) *dy = g_dir_dy[d];
}

int nexus_normalize_dir(int d) {
    return ((d % 4) + 4) % 4;
}

int nexus_dir_diff(int from, int to) {
    int diff = to - from;
    diff = ((diff % 4) + 4) % 4;
    return diff;
}

/* ═══════════════════════════════════════════════════════════════════
 * Square query helpers
 * DM1 square type interpretation (DM1 SQUARE_ELEMENT_TYPE):
 *   0  = solid wall (no passage)
 *   1  = corridor / normal floor
 *   2  = stairs down
 *   3  = stairs up
 *   4  = door (passable when open)
 *   5  = teleporter
 *   6  = alarm trap (set all creatures alerted)
 *   7  = chute/trapdoor (force party to next level)
 *   8-14 = various floor types, water, fire, etc.
 * Source: DM1 DUNGEON.C SQUARE_TYPE() macro, DUNGEON_SQUARE_MASK_TYPE.
 * Nexus uses same lower-5-bits encoding (nexus_v1_dungeon.c:rb16 & 0x1F).
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_square_is_stairs(int sq) {
    /* DM1: type 2 = stairs down, type 3 = stairs up */
    return sq == 2 || sq == 3;
}

int nexus_square_is_pit(int sq) {
    /* DM1: type 13 (NEXUS_SQUARE_CHUTE) is the chute/trapdoor square.
     * Pit and chute are the same effect (force party to next level).
     * Source: DM1 DUNGEON.C SQUARE_TYPE(), nexus_squares.h NEXUS_SQUARE_CHUTE. */
    return sq == 13;
}

int nexus_square_is_teleporter(int sq) {
    /* DM1: type 9 = teleporter, type 10 = teleporter 2, type 11 = teleporter 3.
     * Nexus adds type 11 (NEXUS_SQUARE_TELEPORT3 — intra-level teleporter).
     * Source: DM1 DUNGEON.C F0267/F0268, nexus_squares.h NEXUS_SQUARE_TELEPORT3. */
    return sq == 9 || sq == 10 || sq == 11;
}

int nexus_square_is_door(int sq) {
    /* Nexus door square is type 8 (3D door geometry, scripted via SDDRVS.TSK).
     * Source: nexus_v1_movement.h NEXUS_SQUARE_DOOR, nexus_squares.h. */
    return sq == 8;
}

int nexus_square_is_water(int sq) {
    /* Type 21 = water square (needs rope to cross).
     * STUB: in V1, water blocks passage until rope item is used.
     * Source: DM1 water square behavior (NEXUS_SQUARE_WATER). */
    return sq == 21;
}

int nexus_square_is_fire(int sq) {
    /* Type 22 = fire square (needs protect-from-fire rune).
     * STUB: in V1, fire blocks passage until PROTECT rune is cast.
     * Source: DM1 fire square behavior (NEXUS_SQUARE_FIRE). */
    return sq == 22;
}

int nexus_square_is_passable(int sq) {
    /* Wall (type 0) is impassable. Water (21) and fire (22) are also
     * impassable in V1 (require item/rune to cross).
     * Source: DM1 DUNGEON.C SQUARE_TYPE() passability. */
    if (sq == 0 || sq == 21 || sq == 22) return 0;
    return 1;
}

/* Get square type from dungeon grid */
int nexus_get_square(const uint8_t squares[32][32], int x, int y) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return 0; /* wall = out of bounds */
    return squares[y][x] & 0x1F;
}

/* ═══════════════════════════════════════════════════════════════════
 * Target square computation
 * CLIKMENU.C:224-233 arrow deltas: command-to-step mapping
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_target_square(int map_x, int map_y, int dir,
                         int forward, int strafe,
                         int strafe_left,
                         int *out_x, int *out_y) {
    int dx, dy, d;

    d = ((dir % 4) + 4) % 4;

    if (strafe) {
        /* Strafe: perpendicular to facing direction */
        /* strafe_left = 1: to the left of facing direction */
        /* strafe_left = 0: to the right of facing direction */
        int left = (d + 3) & 3;
        int right = (d + 1) & 3;
        int side_dir = strafe_left ? left : right;
        dx = g_dir_dx[side_dir];
        dy = g_dir_dy[side_dir];
    } else if (forward) {
        /* Forward step in facing direction */
        dx = g_dir_dx[d];
        dy = g_dir_dy[d];
    } else {
        /* Backward step in opposite direction */
        dx = -g_dir_dx[d];
        dy = -g_dir_dy[d];
    }

    if (out_x) *out_x = map_x + dx;
    if (out_y) *out_y = map_y + dy;
    return 0;
}

/* Apply turn */
void nexus_turn(int *in_out_dir, int turn_right) {
    int d;
    if (!in_out_dir) return;
    d = *in_out_dir;
    d = turn_right ? (d + 1) : (d + 3);
    *in_out_dir = d & 3;
}

/* ═══════════════════════════════════════════════════════════════════
 * Core movement resolution — mirrors DM1 F0267/F0268
 * Source: MOVESENS.C F0267 lines 316-328, 752-783:
 *   party-square/scent/last-movement update,
 *   walk-off/walk-on sensor processing,
 *   cross-map new-party-map deferral.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_try_move(int dir, int forward,
                   const uint8_t squares[32][32],
                   int *in_out_map_x, int *in_out_map_y,
                   int *out_result,
                   int *out_new_map_x, int *out_new_map_y) {
    int target_x, target_y, sq;

    if (!in_out_map_x || !in_out_map_y) return 0;

    /* Compute target square based on direction and forward/backward */
    nexus_target_square(*in_out_map_x, *in_out_map_y, dir,
                         forward, 0, 0, &target_x, &target_y);

    sq = nexus_get_square(squares, target_x, target_y);

    /* Check if target is solid wall */
    if (sq == 0) {
        if (out_result) *out_result = NEXUS_MOVE_BLOCKED_WALL;
        if (out_new_map_x) *out_new_map_x = *in_out_map_x;
        if (out_new_map_y) *out_new_map_y = *in_out_map_y;
        return 0;
    }

    /* Check door — all doors treated as open in V1 movement
     * (door state managed at higher level via SDDRVS.TSK) */
    if (sq == 8) {
        /* Door square (type 8) — passable when open.
         * Door state (open/closed/locked) tracked in door registry.
         * V1 movement treats all doors as open by default.
         * Source: nexus_v1_squares.c nexus_doors_register(), SDDRVS.TSK. */
    }

    /* Check special squares */
    if (sq == 2) {
        /* Stairs down */
        *in_out_map_x = target_x;
        *in_out_map_y = target_y;
        if (out_new_map_x) *out_new_map_x = target_x;
        if (out_new_map_y) *out_new_map_y = target_y;
        if (out_result) *out_result = NEXUS_MOVE_STAIRS_DOWN;
        return 1;
    }
    if (sq == 3) {
        /* Stairs up */
        *in_out_map_x = target_x;
        *in_out_map_y = target_y;
        if (out_new_map_x) *out_new_map_x = target_x;
        if (out_new_map_y) *out_new_map_y = target_y;
        if (out_result) *out_result = NEXUS_MOVE_STAIRS_UP;
        return 1;
    }
    /* Check water (type 21) and fire (type 22) — impassable in V1
     * until rope/protect-rune is used.
     * Source: nexus_squares.h NEXUS_SQUARE_WATER/FIRE stub. */
    if (sq == 21) {
        if (out_result) *out_result = NEXUS_MOVE_BLOCKED_WALL;
        if (out_new_map_x) *out_new_map_x = *in_out_map_x;
        if (out_new_map_y) *out_new_map_y = *in_out_map_y;
        return 0;
    }
    if (sq == 22) {
        if (out_result) *out_result = NEXUS_MOVE_BLOCKED_WALL;
        if (out_new_map_x) *out_new_map_x = *in_out_map_x;
        if (out_new_map_y) *out_new_map_y = *in_out_map_y;
        return 0;
    }

    /* Check chute/pit (type 13) — party forced to next level.
     * Movement succeeds — chute is a level-change trigger.
     * Source: nexus_squares.h NEXUS_SQUARE_CHUTE (13). */
    if (sq == 13) {
        *in_out_map_x = target_x;
        *in_out_map_y = target_y;
        if (out_new_map_x) *out_new_map_x = target_x;
        if (out_new_map_y) *out_new_map_y = target_y;
        if (out_result) *out_result = NEXUS_MOVE_FELL_IN_PIT;
        return 1;
    }
    if (sq == 9 || sq == 10 || sq == 11) {
        /* Teleporter (types 9/10/11).
         * Source: nexus_squares.h NEXUS_SQUARE_TELEPORT/TELEPORT2/TELEPORT3. */
        *in_out_map_x = target_x;
        *in_out_map_y = target_y;
        if (out_new_map_x) *out_new_map_x = target_x;
        if (out_new_map_y) *out_new_map_y = target_y;
        if (out_result) *out_result = NEXUS_MOVE_TELEPORTED;
        return 1;
    }

    /* Normal floor/corridor — move succeeds */
    *in_out_map_x = target_x;
    *in_out_map_y = target_y;
    if (out_new_map_x) *out_new_map_x = target_x;
    if (out_new_map_y) *out_new_map_y = target_y;
    if (out_result) *out_result = NEXUS_MOVE_OK;
    return 1;
}

/* Stamina cost formula — matches DM1 MOVESENS.C:597.
 * Source: MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE line 597:
 *   F0325_CHAMPION_DecrementStamina(..., ((load * 25) / max_load) + 1);
 * NOTE: The multiplier is 25, not 3. The load*3 version was a local
 * stub error — the correct DM1 formula uses load*25/max_load+1.
 * BUG FIX: was load*3 (stub error), corrected to load*25 per ReDMCSB.
 * ReDMCSB: MOVESENS.C F0267 line 597, CHAMPION.C F0325 line 2025. */
int nexus_movement_stamina_cost(int load, int max_load) {
    int max_l = max_load > 0 ? max_load : 1;
    return ((load * 25) / max_l) + 1;
}

/* ═══════════════════════════════════════════════════════════════════
 * Movement state — per-game-tick processing
 * Source: CLIKMENU.C F0366 lines 264-276 stairs special,
 *         269-323 move-result + cooldown assignment.
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_movement_init(Nexus_MovementState *ms, int start_x, int start_y, int start_dir) {
    if (!ms) return;
    memset(ms, 0, sizeof(*ms));
    nexus_input_queue_init(&ms->queue);
    ms->party_x = start_x;
    ms->party_y = start_y;
    ms->party_dir = start_dir;
    ms->level_index = 0;
    ms->move_cooldown_ticks = 0;
    ms->last_move_time = 0;
}

void nexus_movement_tick(Nexus_MovementState *ms,
                          const uint8_t squares[32][32],
                          int *out_cmd,
                          Nexus_MoveResultData *out_result) {
    int cmd = 0;

    if (!ms) return;

    /* Decrement movement cooldown */
    if (ms->move_cooldown_ticks > 0) {
        ms->move_cooldown_ticks--;
    }

    /* Try to dequeue a command */
    if (ms->move_cooldown_ticks == 0) {
        if (!nexus_input_queue_pop(&ms->queue, &cmd)) {
            cmd = NEXUS_CMD_NONE;
        }
    } else {
        cmd = NEXUS_CMD_NONE;
    }

    if (out_cmd) *out_cmd = cmd;

    if (cmd == NEXUS_CMD_NONE) {
        if (out_result) memset(out_result, 0, sizeof(*out_result));
        return;
    }

    /* Dispatch command */
    if (cmd == NEXUS_CMD_TURN_LEFT) {
        nexus_movement_turn(ms, 0, out_result);
    } else if (cmd == NEXUS_CMD_TURN_RIGHT) {
        nexus_movement_turn(ms, 1, out_result);
    } else {
        int r = nexus_movement_step(ms, squares, cmd, out_result);
        (void)r;
    }
}

int nexus_movement_step(Nexus_MovementState *ms,
                        const uint8_t squares[32][32],
                        int cmd,
                        Nexus_MoveResultData *out_result) {
    int dir, forward, success;

    if (!ms || !out_result) return 0;
    memset(out_result, 0, sizeof(*out_result));

    dir = ms->party_dir;
    forward = (cmd == NEXUS_CMD_FORWARD) ? 1 : 0;

    if (cmd == NEXUS_CMD_BACKWARD) forward = 0;

    out_result->result = NEXUS_MOVE_BLOCKED_WALL;
    out_result->new_map_x = ms->party_x;
    out_result->new_map_y = ms->party_y;
    out_result->new_dir = ms->party_dir;
    out_result->new_map_index = ms->level_index;

    success = nexus_try_move(dir, forward, squares,
                              &ms->party_x, &ms->party_y,
                              &out_result->result,
                              &out_result->new_map_x, &out_result->new_map_y);

    if (success && out_result->result == NEXUS_MOVE_OK) {
        /* Movement succeeded — set cooldown
         * DM1: ~330ms between steps (6 VBlanks at 55ms each = 330ms)
         * Source: CLIKMENU.C:325-328 G0310 cooldown assignment */
        ms->move_cooldown_ticks = 6;
        return 1;
    } else if (success && out_result->result != NEXUS_MOVE_BLOCKED_WALL) {
        /* Special: stairs/teleport/pit — no cooldown (immediate transition) */
        return 1;
    }

    /* Blocked — no cooldown advance, command discarded
     * Source: CLIKMENU.C:269-323 input discard + blocked-movement VBlank */
    return 0;
}

void nexus_movement_turn(Nexus_MovementState *ms, int turn_right,
                          Nexus_MoveResultData *out_result) {
    if (!ms) return;
    memset(out_result, 0, sizeof(*out_result));

    nexus_turn(&ms->party_dir, turn_right);

    out_result->result = NEXUS_MOVE_TURN_ONLY;
    out_result->new_map_x = ms->party_x;
    out_result->new_map_y = ms->party_y;
    out_result->new_dir = ms->party_dir;
    out_result->new_map_index = ms->level_index;
}