#ifndef NEXUS_V1_SQUARES_H
#define NEXUS_V1_SQUARES_H

#include <stdint.h>

/* Nexus V1 special square processing — doors, pits, teleporters,
 * stairs, traps. Mirrors DM1 DUNGEON.C / MOVESENS.C sensor processing.
 * Source: DM1 DUNGEON.C square type dispatch, MOVESENS.C F0267/F0268,
 * docs/nexus_squares.md. Nexus SDDRVS.TSK replaces sensor array but
 * square-type effects are unchanged from DM1. */

/* Square type constants (lower 5 bits of square byte, matches DM1).
 * Source: DM1 DUNGEON.C square type dispatch, MOVESENS.C F0267/F0268,
 * nexus_v1_movement.h (shared NEXUS_SQUARE_* values). */
#define NEXUS_SQUARE_WALL       0
#define NEXUS_SQUARE_FLOOR      1
#define NEXUS_SQUARE_STAIRS_DN   2
#define NEXUS_SQUARE_STAIRS_UP   3
#define NEXUS_SQUARE_DOOR       8
#define NEXUS_SQUARE_TELEPORT   9
#define NEXUS_SQUARE_TELEPORT2  10
#define NEXUS_SQUARE_TELEPORT3  11
#define NEXUS_SQUARE_ALARM      12
#define NEXUS_SQUARE_CHUTE      13
#define NEXUS_SQUARE_EXIT       14
#define NEXUS_SQUARE_WATER      21
#define NEXUS_SQUARE_FIRE       22
#define NEXUS_SQUARE_13         13
#define NEXUS_SQUARE_14         14

/* Square info */
typedef struct {
    int type;
    int passable;        /* 1 = can move into, 0 = solid */
    int is_door;         /* 1 = door square */
    int is_stairs;       /* 1 = stairs */
    int is_teleporter;   /* 1 = teleporter */
    int is_trap;         /* 1 = pit/alarm/chute/trap */
    int is_exit;         /* 1 = dungeon exit */
    int teleporter_target; /* target square index for teleport (0-15) */
} Nexus_SquareInfo;

const char *nexus_square_type_name(int type);
int nexus_square_is_passable_type(int type);
int nexus_square_is_wall(int type);

/* Get full square info from type */
Nexus_SquareInfo nexus_square_info(int type);

/* Door state management — used by nexus_doors_* API.
 * The NEXUS_DOOR_* #define constants are in nexus_v1_movement.h
 * (shared across movement and square systems).
 * Source: DM1 DUNGEON.C door state, nexus_doors_register() in nexus_v1_squares.c. */
#define NEXUS_DOOR_STATE_CLOSED 0
#define NEXUS_DOOR_STATE_OPEN   1
#define NEXUS_DOOR_STATE_LOCKED  2

#define NEXUS_MAX_DOORS 64

typedef struct {
    int x, y;
    uint8_t door_state;  /* NEXUS_DOOR_STATE_* — open/closed/locked */
    int key_required;     /* item_id of key, or -1 */
} Nexus_Door;

void nexus_doors_init(void);
int nexus_doors_open(int x, int y);
int nexus_doors_close(int x, int y);
int nexus_doors_toggle(int x, int y);
int nexus_doors_lock(int x, int y, int key_item_id);
int nexus_doors_is_open(int x, int y);
int nexus_doors_can_open(int x, int y, const uint8_t inventory[30]);
int nexus_doors_register(int x, int y);

/* Teleporter resolution */
typedef struct {
    int from_x, from_y;
    int to_x, to_y;
    int to_level;
} Nexus_TeleporterLink;

#define NEXUS_MAX_TELEPORTERS 64

void nexus_teleporters_init(void);
int nexus_teleporters_register(int from_x, int from_y, int to_x, int to_y, int to_level);
int nexus_teleporters_resolve(int x, int y, int *out_to_x, int *out_to_y, int *out_to_level);
int nexus_teleporters_count(void);

/* Stairs resolution */
typedef struct {
    int x, y;
    int target_level;    /* -1 = up, +1 = down (or specific level) */
    int target_x, target_y;
    int target_dir;       /* direction party faces after stairs */
} Nexus_StairsLink;

#define NEXUS_MAX_STAIRS 32

void nexus_stairs_init(void);
int nexus_stairs_register(int x, int y, int target_level, int target_x, int target_y, int target_dir);
int nexus_stairs_resolve(int x, int y, int *out_target_level, int *out_target_x, int *out_target_y, int *out_target_dir);
int nexus_stairs_count(void);

/* Square event — returned when party steps on a square.
 * Caller dispatches based on event type. */
typedef enum {
    NEXUS_EVENT_NONE = 0,
    NEXUS_EVENT_MOVE_OK,
    NEXUS_EVENT_BLOCKED,
    NEXUS_EVENT_STAIRS_UP,
    NEXUS_EVENT_STAIRS_DOWN,
    NEXUS_EVENT_DOOR_OPEN,
    NEXUS_EVENT_DOOR_BLOCKED,
    NEXUS_EVENT_PIT_FALL,
    NEXUS_EVENT_TELEPORT,
    NEXUS_EVENT_ALARM_TRIGGER,
    NEXUS_EVENT_CHUTE_FALL,
    NEXUS_EVENT_EXIT_REACHED
} Nexus_SquareEvent;

/* Process square event when party enters a square */
Nexus_SquareEvent nexus_process_square_event(int type, int x, int y,
                                               int *out_target_x, int *out_target_y,
                                               int *out_target_level, int *out_target_dir);

/* Check if square triggers an event on entry */
int nexus_square_triggers_on_entry(int type);

#endif /* NEXUS_V1_SQUARES_H */