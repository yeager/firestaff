#ifndef NEXUS_V1_SQUARES_H
#define NEXUS_V1_SQUARES_H

#include <stdint.h>

struct Nexus_V1_Champion;  /* forward declaration for altar ritual API */

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
 * Source: DM1 DUNGEON.C door state, nexus_doors_register() in nexus_v1_squares.c.
 *
 * Door animation: DM1 doors animate through several stepped positions when
 * opening/closing.  We model this with intermediate OPENING/CLOSING states
 * and an animation_step counter (0..NEXUS_DOOR_ANIMATION_STEPS).  A door is
 * fully passable only once the step reaches the threshold.
 * Source: DM1 viewport door animation (SDDRVS.TSK / renderer door overlay). */
#define NEXUS_DOOR_STATE_CLOSED   0
#define NEXUS_DOOR_STATE_OPEN     1
#define NEXUS_DOOR_STATE_LOCKED   2
#define NEXUS_DOOR_STATE_OPENING  3
#define NEXUS_DOOR_STATE_CLOSING  4

#define NEXUS_DOOR_ANIMATION_STEPS        4
#define NEXUS_DOOR_PASSABLE_STEP_THRESHOLD 2

#ifndef NEXUS_MAX_DOORS
#define NEXUS_MAX_DOORS 64
#endif

typedef struct {
    int x, y;
    uint8_t door_state;  /* NEXUS_DOOR_STATE_* — open/closed/locked/animating */
    int key_required;     /* item_id of key, or -1 */
    int animation_step;   /* 0..NEXUS_DOOR_ANIMATION_STEPS */
} Nexus_SquareDoor;

void nexus_doors_init(void);
int nexus_doors_open(int x, int y);
int nexus_doors_close(int x, int y);
int nexus_doors_toggle(int x, int y);
int nexus_doors_lock(int x, int y, int key_item_id);
int nexus_doors_is_open(int x, int y);
int nexus_doors_is_passable(int x, int y);
int nexus_doors_can_open(int x, int y, const uint8_t inventory[30]);
int nexus_doors_register(int x, int y);

/* Advance door animation by one tick.  Returns 1 if any door changed state. */
int nexus_doors_tick_animation(void);

/* Return current animation step (0..NEXUS_DOOR_ANIMATION_STEPS) or 0. */
int nexus_doors_animation_step(int x, int y);

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

/* Pit / chute resolution
 * DM1 MOVESENS.C F0267/F0268: chute squares force a level transition to a
 * registered target cell.  When no real target is registered the default is
 * one level down at the same (x,y).
 * Source: DMWeb DGN Structure1F floor-sensor destination fields. */
#define NEXUS_MAX_PITS 64

typedef struct {
    int x, y;
    int target_x, target_y;
    int target_level;    /* -1 = default one level down */
} Nexus_PitLink;

void nexus_pits_init(void);
int nexus_pits_register(int x, int y, int target_x, int target_y, int target_level);
int nexus_pits_resolve(int x, int y, int *out_target_x, int *out_target_y, int *out_target_level);
int nexus_pits_count(void);

/* Altar registry — Vi altar of rebirth (alcove-based resurrection).
 * Source: DMWeb DGN Structure1Fd docs, Nexus manual translation.
 * Nexus has only the Vi altar; FUL/BRO/GATH are DM1-specific and do not exist. */
#define NEXUS_MAX_ALTARS 64

#define NEXUS_ALTAR_TAG_UNKNOWN 0
#define NEXUS_ALTAR_TAG_VI      1

typedef struct {
    int x, y;
    uint8_t tag;
    int level;
} Nexus_Altar;

void nexus_altars_init(void);
int nexus_altars_register(int x, int y);
int nexus_altars_register_tagged(int x, int y, uint8_t tag);
int nexus_altar_at(int x, int y);
int nexus_altar_tag_at(int x, int y);
int nexus_altars_count(void);

int nexus_altar_resurrect(int x, int y, struct Nexus_V1_Champion *champion);
int nexus_altar_perform_ritual(int x, int y, struct Nexus_V1_Champion *leader);

/* Registry counts (used by probes/tests) */
int nexus_doors_count(void);

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
    NEXUS_EVENT_EXIT_REACHED,
    NEXUS_EVENT_CROSS_WATER,   /* party crossed water square (rope required) */
    NEXUS_EVENT_CROSS_FIRE     /* party crossed fire square (fire rune required) */
} Nexus_SquareEvent;

/* Process square event when party enters a square */
Nexus_SquareEvent nexus_process_square_event(int type, int x, int y,
                                               int *out_target_x, int *out_target_y,
                                               int *out_target_level, int *out_target_dir);

/* Check if square triggers an event on entry */
int nexus_square_triggers_on_entry(int type);

#endif /* NEXUS_V1_SQUARES_H */
