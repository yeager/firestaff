#ifndef NEXUS_V1_MOVEMENT_H
#define NEXUS_V1_MOVEMENT_H

#include <stdint.h>
#include "nexus_v1_dungeon.h"

/* Nexus V1 party movement — DM1-compatible grid movement on Nexus maps.
 * Source: DM1 CLIKMENU.C / MOVESENS.C movement pipeline;
 * docs/nexus_input.md (keyboard mapping); ReDMCSB COMMAND.C/F0380.
 *
 * Commands mirror DM1: forward/backward step (relative to facing),
 * turn left/right 90°, strafe left/right.
 *
 * Note: Nexus_MoveCommand, Nexus_MoveResult, Nexus_MoveResultData,
 * Nexus_InputQueue, Nexus_InputCommand, and Nexus_PartyState are also
 * defined here and must not be redefined by other headers.
 * Command-type and result-type enums live HERE only. */

// Direction constants (0=N, 1=E, 2=S, 3=W)
#define NEXUS_DIR_NORTH  0
#define NEXUS_DIR_EAST   1
#define NEXUS_DIR_SOUTH  2
#define NEXUS_DIR_WEST   3

#ifndef NEXUS_CMD_NONE
#define NEXUS_CMD_NONE          0
#define NEXUS_CMD_FORWARD       1
#define NEXUS_CMD_BACKWARD      2
#define NEXUS_CMD_TURN_LEFT     3
#define NEXUS_CMD_TURN_RIGHT    4
#define NEXUS_CMD_STRAFE_LEFT   5
#define NEXUS_CMD_STRAFE_RIGHT  6
#define NEXUS_CMD_OPEN_DOOR     7
#define NEXUS_CMD_USE_ITEM      8
#define NEXUS_CMD_CAST_SPELL    9
#define NEXUS_CMD_TOGGLE_MAP    10
#define NEXUS_CMD_COUNT         11
#endif

/* Movement result codes — mirrors DM1 MOVESENS result values.
 * Source: DM1 CLIKMENU.C F0366 COMMAND_ProcessTypes3To6_MoveParty,
 *         MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE.
 * Must match the int result field of Nexus_MoveResultData below. */
#define NEXUS_MOVE_OK            0
#define NEXUS_MOVE_BLOCKED_WALL  1
#define NEXUS_MOVE_STAIRS_DOWN   2
#define NEXUS_MOVE_STAIRS_UP     3
#define NEXUS_MOVE_FELL_IN_PIT   4
#define NEXUS_MOVE_TELEPORTED    5
#define NEXUS_MOVE_TURN_ONLY     6

/* Square type flags — extended bitfield for rendering/effects.
 * Low 5 bits = square type, upper bits = flags.
 * Source: nexus_v1_dungeon.c:rb16 & 0x1F (square type), extended bits
 * inferred from DM1 SQUARE_EFFECT flags (DM1 DUNGEON.C). */
#define NEXUS_SQUARE_WALL       0
#define NEXUS_SQUARE_FLOOR      1
#define NEXUS_SQUARE_STAIRS_DN   2
#define NEXUS_SQUARE_STAIRS_UP   3
#define NEXUS_SQUARE_DOOR       8   /* 3D door geometry, scripted via SDDRVS.TSK */
#define NEXUS_SQUARE_TELEPORT   9
#define NEXUS_SQUARE_TELEPORT2  10
#define NEXUS_SQUARE_TELEPORT3  11
#define NEXUS_SQUARE_ALARM      12
#define NEXUS_SQUARE_CHUTE      13
#define NEXUS_SQUARE_EXIT       14
#define NEXUS_SQUARE_WATER      21  /* needs rope to cross */
#define NEXUS_SQUARE_FIRE       22  /* needs protect-from-fire rune */

/* Extended square flags (upper 3 bits of byte) */
#define NEXUS_SQF_EXTENDED      0x20
#define NEXUS_SQF_SECRET        0x40
#define NEXUS_SQF_3D_ONLY       0x80

/* Door state bits (used with NEXUS_SQUARE_DOOR) */
#define NEXUS_DOOR_CLOSED       0
#define NEXUS_DOOR_OPEN         1
#define NEXUS_DOOR_LOCKED       2

/* Movement result structure */
typedef struct {
    int result;  /* NEXUS_MOVE_* */
    int new_map_x, new_map_y;
    int new_dir;
    int new_map_index;
} Nexus_MoveResultData;

#define NEXUS_INPUT_QUEUE_SIZE 8

typedef struct {
    int command;
    int dequeued;
} Nexus_InputCommand;

typedef struct {
    Nexus_InputCommand commands[NEXUS_INPUT_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} Nexus_InputQueue;

/* Party state for movement */
typedef struct {
    int map_x, map_y;     /* current position */
    int direction;         /* 0=N, 1=E, 2=S, 3=W */
    int map_index;         /* level index 0-15 */
} Nexus_PartyState;

/* ═══════════════════════════════════════════════════════════════════
 * Map coord helpers
 * ═══════════════════════════════════════════════════════════════════ */
void nexus_dir_deltas(int dir, int *dx, int *dy);
int nexus_normalize_dir(int d);
int nexus_dir_diff(int from, int to);  /* signed difference modulo 4 */

/* ═══════════════════════════════════════════════════════════════════
 * Input queue management
 * ═══════════════════════════════════════════════════════════════════ */
void nexus_input_queue_init(Nexus_InputQueue *q);
int nexus_input_queue_push(Nexus_InputQueue *q, int command);
int nexus_input_queue_pop(Nexus_InputQueue *q, int *out_cmd);
int nexus_input_queue_count(const Nexus_InputQueue *q);
void nexus_input_queue_discard(Nexus_InputQueue *q);

/* ═══════════════════════════════════════════════════════════════════
 * Movement resolution
 * ═══════════════════════════════════════════════════════════════════ */

/* Attempt to move party one step. Returns result + updated coords.
 * blocking_square_type: 0=wall (solid), non-zero=passable.
 * door_square: 1=open, 0=closed.
 * Source: DM1 CLIKMENU.C F0366 COMMAND_ProcessTypes3To6_MoveParty
 *         MOVESENS.C F0267 F0268 movement resolution */
int nexus_try_move(int dir, int forward,
                   const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                   int *in_out_map_x, int *in_out_map_y,
                   int *out_result,
                   int *out_new_map_x, int *out_new_map_y);

/* Apply turn command (left/right). Updates direction. */
void nexus_turn(int *in_out_dir, int turn_right);

/* Compute target square for a movement command.
 * forward=1: forward step; forward=0: backward step.
 * strafe=1: lateral step (left/right from current dir). */
int nexus_target_square(int map_x, int map_y, int dir,
                        int forward, int strafe,
                        int strafe_left,
                        int *out_x, int *out_y);

/* Get square type at map coordinates */
int nexus_get_square(const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE], int x, int y);

/* ═══════════════════════════════════════════════════════════════════
 * Movement tick — call nexus_v1_tick_movement() each 55ms game tick
 * ═══════════════════════════════════════════════════════════════════ */
typedef struct {
    Nexus_InputQueue queue;
    int disabled_ticks;         /* cooldown ticks */
    int party_x, party_y, party_dir;
    int level_index;
    int move_cooldown_ticks;    /* movement disabled during this many ticks */
    unsigned long last_move_time;
} Nexus_MovementState;

void nexus_movement_init(Nexus_MovementState *ms, int start_x, int start_y, int start_dir);
void nexus_movement_tick(Nexus_MovementState *ms,
                          const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                          int *out_cmd,
                          Nexus_MoveResultData *out_result);
int nexus_movement_step(Nexus_MovementState *ms,
                        const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                        int cmd,
                        Nexus_MoveResultData *out_result);
void nexus_movement_turn(Nexus_MovementState *ms, int turn_right,
                          Nexus_MoveResultData *out_result);

/* Stair/teleporter/pit resolution helpers */
int nexus_square_is_stairs(int sq);
int nexus_square_is_pit(int sq);
int nexus_square_is_teleporter(int sq);
int nexus_square_is_door(int sq);
int nexus_square_is_passable(int sq);

/* ═══════════════════════════════════════════════════════════════════
 * Stamina cost for movement (DM1-compatible)
 * CLIKMENU.C:237-255 applies F0325 stamina cost before movement.
 * ═══════════════════════════════════════════════════════════════════ */
int nexus_movement_stamina_cost(int load, int max_load);

#endif /* NEXUS_V1_MOVEMENT_H */
