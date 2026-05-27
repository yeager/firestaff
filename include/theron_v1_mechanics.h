/*
 * theron_v1_mechanics.h — Theron's Quest V1 Phase 5: Mechanics Parity
 *
 * Movement, click routes, doors, pits, teleporters, altar-of-vi
 * resurrection, and per-move champion stat processing.
 *
 * Source references:
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header + dungeon_seed)
 *   THQUEST.ASM T600  — map transitions + teleporter chains
 *   THQUEST.ASM T700  — tick world / per-tick stat updates
 *   THQUEST.ASM T800  — champion persistence + inventory reset per dungeon
 *   THQUEST.ASM T900  — object database / thing list
 *
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *   docs/source-lock/movement_features.md            (pit/teleporter Chain)
 *   docs/source-lock/movement_forward_step.md
 *   docs/source-lock/movement_collision.md
 */

#ifndef THERON_V1_MECHANICS_H
#define THERON_V1_MECHANICS_H

#include "theron_v1_world.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Movement direction ─────────────────────────────────────────── */
#define THERON_DIR_NORTH  0
#define THERON_DIR_EAST   1
#define THERON_DIR_SOUTH  2
#define THERON_DIR_WEST   3
#define THERON_DIR_COUNT  4

/* ── Door state ──────────────────────────────────────────────────── */
/* TQR door square state machine (mirrors DM1 C0-C5 states) */
#define THERON_DOOR_STATE_CLOSED        0   /* solid barrier */
#define THERON_DOOR_STATE_QUARTER_OPEN  1   /* passable */
#define THERON_DOOR_STATE_HALF_OPEN     2   /* passable */
#define THERON_DOOR_STATE_THREEQUARTER_OPEN 3 /* passable */
#define THERON_DOOR_STATE_OPEN          4   /* fully open */
#define THERON_DOOR_STATE_DESTROYED     5   /* rubble — passable */
#define THERON_DOOR_STATE_LOCKED        6   /* needs key + open */

/* ── Door flags ──────────────────────────────────────────────────── */
#define THERON_DOOR_F_LOCKED    (1U << 0)  /* requires key */
#define THERON_DOOR_F_SECRET   (1U << 1)  /* hidden until triggered */
#define THERON_DOOR_F_BROKEN   (1U << 2)  /* permanently open */
#define THERON_DOOR_F_AUTO_OPEN (1U << 3)  /* opens on step */
#define THERON_DOOR_F_MANUAL    (1U << 4)  /* requires EXAMINE/USE command */

/* ── Directional deltas ───────────────────────────────────────────── */
extern const int8_t g_theron_dir_dx[THERON_DIR_COUNT];
extern const int8_t g_theron_dir_dy[THERON_DIR_COUNT];

/* ── Teleporter chain limits (matches DM1 S21E+) ─────────────────── */
#define THERON_TELEPORTER_CHAIN_MAX  100

/* ── Click/command routes ─────────────────────────────────────────── */
#define THERON_CMD_MOVE     1
#define THERON_CMD_LOOK     2
#define THERON_CMD_USE      3
#define THERON_CMD_TAKE     4
#define THERON_CMD_ATTACK   5
#define THERON_CMD_DROP     6
#define THERON_CMD_EXAMINE  7
/*
 * theron_v1_click_route — resolve a mouse/touch click at grid (x,y)
 *   world:    IN/OUT game world
 *   x, y:     grid coordinates clicked
 *   command:  THERON_CMD_MOVE / THERON_CMD_LOOK / THERON_CMD_USE /
 *             THERON_CMD_TAKE / THERON_CMD_ATTACK / THERON_CMD_DROP /
 *             THERON_CMD_EXAMINE
 *   return:   0 on success, negative on error / blocked
 */
int theron_v1_click_route(Theron_V1_World *world, int x, int y, int command);

/* ── Movement ────────────────────────────────────────────────────────── */
/*
 * theron_v1_move_party — attempt to move party forward one square
 *   world:     IN/OUT game world
 *   direction: 0=N 1=E 2=S 3=W
 *   return:    0 blocked (wall/closed door/spec), 1 moved, 2 special square
 *
 * theron_v1_turn_party — rotate party left or right
 *   world:     IN/OUT
 *   turn:      -1 = left 90°, +1 = right 90°
 */
int theron_v1_move_party(Theron_V1_World *world, int direction);
int theron_v1_turn_party(Theron_V1_World *world, int turn);

/* ── Movement result detail ────────────────────────────────────────── */
typedef enum {
    THERON_MOVE_OK          = 0,  /* normal floor move */
    THERON_MOVE_BLOCKED     = 1,  /* wall / closed door */
    THERON_MOVE_SPECIAL     = 2,  /* landed on special square (auto-handled) */
    THERON_MOVE_PIT_FALL   = 3,  /* pit open — party fell */
    THERON_MOVE_TELEPORT    = 4,  /* teleporter activated */
    THERON_MOVE_STAIRS      = 5,  /* stairs taken */
    THERON_MOVE_EXIT        = 6,  /* dungeon exit reached */
} Theron_MoveResult;

/* theron_v1_get_move_result — query result of hypothetical move */
Theron_MoveResult theron_v1_get_move_result(const Theron_V1_World *world,
                                            int direction);

/* ── Door interactions ─────────────────────────────────────────────── */
int  theron_v1_door_open(Theron_V1_World *world, int x, int y);
int  theron_v1_door_close(Theron_V1_World *world, int x, int y);
int  theron_v1_door_is_open(const Theron_V1_World *world, int x, int y);
int  theron_v1_door_is_locked(const Theron_V1_World *world, int x, int y);
int  theron_v1_door_unlock_with_key(Theron_V1_World *world, int x, int y, int key_item);

/* ── Pit mechanics ─────────────────────────────────────────────────── */
/*
 * theron_v1_pit_check_and_trigger — called after move onto a square.
 *   Returns true if party fell; updates world state.
 *   Source: movement_features.md — pit OPEN-bit fall chain (ReDMCSB MOVESENS.C:538)
 */
bool theron_v1_pit_check_and_trigger(Theron_V1_World *world,
                                      int x, int y);

/* ── Teleporter chain ─────────────────────────────────────────────── */
/*
 * theron_v1_teleporter_resolve — resolve a teleporter square;
 *   follows chains up to THERON_TELEPORTER_CHAIN_MAX iterations.
 *   Source: movement_features.md — MOVESENS.C:475-537 teleporter loop
 *
 * On return, world->transition_target_level / spawn_x / spawn_y are set.
 */
int theron_v1_teleporter_resolve(Theron_V1_World *world, int x, int y);

/* ── Altar of Vi resurrection ─────────────────────────────────────── */
/*
 * theron_v1_altar_of_vi_resurrect — process altar-of-vi on party landing.
 *   Revives a dead champion if:
 *     1. Champion slot is dead (health == 0)
 *     2. No anti-magic field blocks revival
 *     3. Party carries enough gold (cost = 500 gold per champion)
 *   Source: THQUEST.ASM T900 altar processing; altar-of-vi is unique to TQR
 *
 * Returns: champion slot revived, or -1 if no resurrection available.
 */
int theron_v1_altar_of_vi_resurrect(Theron_V1_World *world, int altar_x, int altar_y);

/* ── Per-move stat updates ─────────────────────────────────────────── */
/*
 * theron_v1_apply_post_move_effects — stamina, wounds, poison, food/water.
 *   Called after every successful move (or move blocked by wall).
 *   Source: THQUEST.ASM T700 / per-tick stat logic.
 */
void theron_v1_apply_post_move_effects(Theron_V1_World *world);

/* ── Pool / recovery square ─────────────────────────────────────────── */
int theron_v1_pool_use(Theron_V1_World *world, int x, int y);

/* ── Alarm trigger ────────────────────────────────────────────────── */
int theron_v1_alarm_trigger(Theron_V1_World *world, int x, int y);

/* ── Trigger / event square ────────────────────────────────────────── */
int theron_v1_trigger_activate(Theron_V1_World *world, int x, int y);

/* ── Source citation ───────────────────────────────────────────────── */
const char *theron_v1_mechanics_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_MECHANICS_H */
