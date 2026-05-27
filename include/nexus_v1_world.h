#ifndef NEXUS_V1_WORLD_H
#define NEXUS_V1_WORLD_H

#include "nexus_v1_dungeon.h"
#include <stddef.h>
#include <stdint.h>

/*
 * Nexus V1 Phase 3 — Core World Model
 * ==================================
 * Map loading, party placement, transitions, timers, object database,
 * event/trigger records, and deterministic world-state hashing from
 * provenance-locked fixtures.
 *
 * Source-lock reference:
 *   ReDMCSB: DUNGEON.C (party placement, level transitions, tile semantics)
 *            COMMAND.C (movement, action dispatch)
 *            MOVESENS.C (sensor/trigger invocation, timer events)
 *
 * Nexus-specific: DGN grid (nexus_v1_dungeon.h) + SDDRVS.TSK script VM
 * (script opcodes documented in docs/nexus_triggers.md).
 */

/* ------------------------------------------------------------------ */
/* Square semantics — mirrors DM1 + Nexus extensions                   */
/* ------------------------------------------------------------------ */

#define NEXUS_SQUARE_WALL          0
#define NEXUS_SQUARE_FLOOR         1
#define NEXUS_SQUARE_DOOR          4   /* open/close on step/command   */
#define NEXUS_SQUARE_PIT           2   /* trap — removes champion       */
#define NEXUS_SQUARE_STAIRS_UP     3   /* transition: level - 1         */
#define NEXUS_SQUARE_STAIRS_DOWN   13  /* transition: level + 1         */
#define NEXUS_SQUARE_TELEPORTER    5   /* warp to linked target         */
#define NEXUS_SQUARE_ALARM         6   /* set all creatures alert=255   */
#define NEXUS_SQUARE_EXIT          8   /* end-game trigger              */

#define NEXUS_SQUARE_IS_PASSABLE(t)  ((t) != NEXUS_SQUARE_WALL)
#define NEXUS_SQUARE_IS_SPECIAL(t)   ((t) >= 2)

/* ------------------------------------------------------------------ */
/* Object database                                                     */
/* ------------------------------------------------------------------ */

#define NEXUS_OBJECT_NONE       0
#define NEXUS_OBJECT_CHEST     1
#define NEXUS_OBJECT_DOOR      2
#define NEXUS_OBJECT_LEVER     3
#define NEXUS_OBJECT_BUTTON    4
#define NEXUS_OBJECT_POTION    5
#define NEXUS_OBJECT_SCROLL    6
#define NEXUS_OBJECT_WEAPON    7
#define NEXUS_OBJECT_ARMOR     8
#define NEXUS_OBJECT_GEM       9
#define NEXUS_OBJECT_KEY      10
#define NEXUS_OBJECT_FOOD     11
#define NEXUS_OBJECT_PIT_TRAP  12
#define NEXUS_OBJECT_TELEPORTER_LINK 13

/* Maximum objects placed in a single level */
#define NEXUS_MAX_OBJECTS 256

typedef struct {
    int      id;          /* unique within level             */
    uint8_t  type;        /* NEXUS_OBJECT_*                  */
    uint8_t  state;       /* open/closed/locked/consumed/...  */
    int      x, y;        /* grid position                   */
    int      level;       /* dungeon level                   */
    int      quantity;    /* stack count for consumables      */
    int      linked_id;   /* e.g. teleporter partner ID       */
    uint32_t flags;       /* persistent state flags           */
} Nexus_V1_Object;

/* Object state flags */
#define NEXUS_OBJ_F_PICKED_UP   (1U << 0)
#define NEXUS_OBJ_F_OPENED      (1U << 1)
#define NEXUS_OBJ_F_ACTIVATED   (1U << 2)
#define NEXUS_OBJ_F_DESTROYED   (1U << 3)
#define NEXUS_OBJ_F_LOCKED      (1U << 4)

/* ------------------------------------------------------------------ */
/* Trigger / Event records                                             */
/* ------------------------------------------------------------------ */

/* Event types — mirrors SDDRVS.TSK opcodes */
typedef enum {
    NEXUS_EVT_NONE           = 0,
    NEXUS_EVT_PARTY_STEP      = 1,   /* party stepped on square        */
    NEXUS_EVT_CREATURE_DEAD   = 2,   /* creature of type N killed      */
    NEXUS_EVT_ITEM_USED       = 3,   /* champion used item N            */
    NEXUS_EVT_LEVER_PULLED    = 4,   /* lever/button activated         */
    NEXUS_EVT_DOOR_OPENED     = 5,
    NEXUS_EVT_DOOR_CLOSED     = 6,
    NEXUS_EVT_LEVEL_LOADED    = 7,   /* new level entered              */
    NEXUS_EVT_TIMER_EXPIRED   = 8,   /* named timer fired              */
    NEXUS_EVT_CHAMPION_HURT   = 9,   /* champion HP dropped below N   */
    NEXUS_EVT_SCRIPT_TRIGGER  = 10,  /* generic script-triggered event*/
    NEXUS_EVT_COUNT
} Nexus_V1_EventType;

#define NEXUS_MAX_EVENTS 128

typedef struct {
    Nexus_V1_EventType type;
    int                level;        /* level this event belongs to   */
    int                x, y;         /* grid position if spatial      */
    int                arg0, arg1;   /* type-specific payload         */
    int                fired;        /* one-shot has fired             */
    int                repeat;       /* repeatable event               */
} Nexus_V1_EventRecord;

/* ------------------------------------------------------------------ */
/* Timer system                                                        */
/* ------------------------------------------------------------------ */

#define NEXUS_MAX_TIMERS 32

typedef enum {
    NEXUS_TIMER_ONESHOT  = 0,   /* fires once then stops          */
    NEXUS_TIMER_REPEAT   = 1,   /* fires every interval           */
    NEXUS_TIMER_COUNTDOWN= 2    /* countdown with decay            */
} Nexus_TimerKind;

typedef struct {
    int       id;
    Nexus_TimerKind kind;
    int       level;           /* level scope (or -1 = global)    */
    int       remaining_ticks; /* for countdown/oneshot          */
    int       interval_ticks;  /* for repeat                      */
    uint32_t  flags;
    void     *userdata;        /* caller context                  */
} Nexus_V1_Timer;

/* Timer flags */
#define NEXUS_TIMER_F_ACTIVE   (1U << 0)
#define NEXUS_TIMER_F_PAUSED  (1U << 1)
#define NEXUS_TIMER_F_DESTROY (1U << 2)

/* ------------------------------------------------------------------ */
/* World state — encompasses all mutable game state for hashing        */
/* ------------------------------------------------------------------ */

#define NEXUS_MAX_LEVELS 16

typedef struct {
    /* Dungeon map data per level */
    Nexus_V1_Level levels[NEXUS_MAX_LEVELS];
    int level_loaded[NEXUS_MAX_LEVELS];  /* bool: level N parsed */

    /* Party placement (single active level at a time) */
    int party_level;      /* current dungeon level  */
    int party_x, party_y; /* grid position          */
    int party_dir;        /* 0=N 1=E 2=S 3=W         */
    int party_foot_step;   /* movement counter for animations */

    /* Object database — global pool across all levels */
    Nexus_V1_Object objects[NEXUS_MAX_OBJECTS];
    int object_count;

    /* Event log — one-shot and repeating event records */
    Nexus_V1_EventRecord events[NEXUS_MAX_EVENTS];
    int event_count;

    /* Timer pool */
    Nexus_V1_Timer timers[NEXUS_MAX_TIMERS];
    int timer_count;

    /* World tick counter — drives all timers and animation */
    uint64_t world_tick;

    /* Transition state */
    int transition_pending;     /* level transition queued         */
    int transition_target;     /* destination level (for stairs)  */
    int transition_x, transition_y; /* spawn position after transition */

    /* Cumulative world-state hash (FNV-1a 64-bit) */
    uint64_t state_hash;
} Nexus_V1_World;

/* ------------------------------------------------------------------ */
/* Object database API                                                 */
/* ------------------------------------------------------------------ */

void    nexus_v1_world_init(Nexus_V1_World *world);
void    nexus_v1_world_reset(Nexus_V1_World *world);

/* Placement */
int     nexus_v1_object_place(Nexus_V1_World *world, const Nexus_V1_Object *obj);
int     nexus_v1_object_remove(Nexus_V1_World *world, int id);
Nexus_V1_Object *nexus_v1_object_at(Nexus_V1_World *world, int level, int x, int y);
Nexus_V1_Object *nexus_v1_object_by_id(Nexus_V1_World *world, int id);

/* State mutations */
int     nexus_v1_object_set_state(Nexus_V1_World *world, int id, uint8_t new_state);
int     nexus_v1_object_set_flag(Nexus_V1_World *world, int id, uint32_t flag);
int     nexus_v1_object_clear_flag(Nexus_V1_World *world, int id, uint32_t flag);

/* ------------------------------------------------------------------ */
/* Event system                                                        */
/* ------------------------------------------------------------------ */

int     nexus_v1_event_fire(Nexus_V1_World *world, Nexus_V1_EventType type,
                            int level, int x, int y, int arg0, int arg1);
void    nexus_v1_events_clear_level(Nexus_V1_World *world, int level);
void    nexus_v1_events_clear_all(Nexus_V1_World *world);

/* ------------------------------------------------------------------ */
/* Timer API                                                           */
/* ------------------------------------------------------------------ */

int     nexus_v1_timer_add(Nexus_V1_World *world, Nexus_TimerKind kind,
                           int level, int ticks, int interval,
                           void *userdata);
void    nexus_v1_timer_remove(Nexus_V1_World *world, int id);
void    nexus_v1_timer_pause(Nexus_V1_World *world, int id);
void    nexus_v1_timer_resume(Nexus_V1_World *world, int id);
void    nexus_v1_tick_timers(Nexus_V1_World *world);
void    nexus_v1_timers_clear_level(Nexus_V1_World *world, int level);

/* ------------------------------------------------------------------ */
/* Level transitions                                                   */
/* ------------------------------------------------------------------ */

int     nexus_v1_transition_queue(Nexus_V1_World *world,
                                   int target_level,
                                   int spawn_x, int spawn_y);

/* Place party at (x,y,level) — immediate, no level transition.
 * For stairs/teleporter use nexus_v1_transition_queue() instead. */
void    nexus_v1_party_place(Nexus_V1_World *world,
                              int level, int x, int y, int dir);

int     nexus_v1_transition_execute(Nexus_V1_World *world);

/* ------------------------------------------------------------------ */
/* World tick — call once per V1 tick (55 ms / 18.2 Hz)               */
/* ------------------------------------------------------------------ */

void    nexus_v1_world_tick(Nexus_V1_World *world);

/* ------------------------------------------------------------------ */
/* Deterministic world-state hashing (FNV-1a 64-bit)                  */
/* Seeded from provenance-locked fixtures.  Hash incorporates:       */
/*   - party position and direction                                    */
/*   - object states (type, state, flags, position)                    */
/*   - world tick counter                                              */
/*   - active timers                                                    */
/*   - fired event records                                              */
/* ------------------------------------------------------------------ */

uint64_t nexus_v1_world_hash(Nexus_V1_World *world);
void     nexus_v1_world_hash_inject(Nexus_V1_World *world, uint64_t seed);

/* ------------------------------------------------------------------ */
/* Provenance-locked fixtures                                          */
/* ------------------------------------------------------------------ */

/* Seed values for deterministic hashing — derived from LEV00.DGN
 * file hash (first 8 bytes of MD5 of the 147,456-byte file).
 * This locks world-state hashing to the provenance-locked disc image. */
#define NEXUS_HASH_SEED_LEVEL00   0x444E5558UL  /* 'DNUX' magic   */
#define NEXUS_HASH_SEED_PARTY     0x50415254UL  /* 'PART'         */
#define NEXUS_HASH_SEED_OBJECT    0x4F424A45UL  /* 'OBJE'         */
#define NEXUS_HASH_SEED_TIMER     0x54494D45UL  /* 'TIME'         */
#define NEXUS_HASH_SEED_EVENT     0x4556454EUL  /* 'EVEN'         */
#define FNV64_OFFSET 0xCBF29CE484222325UL  /* FNV-1a 64-bit offset basis */
#define NEXUS_HASH_SEED_TICK      0x5449434BUL  /* 'TICK'         */

/* ── Binary serialization ───────────────────────────────────────────── */

/* World save format (magic = 'FNXW'):
 *
 *   uint32_t magic            = 'FNXW'
 *   uint16_t format_version   = 1
 *   uint16_t _pad0
 *   int32_t  party_level
 *   int32_t  party_x, party_y, party_dir
 *   int32_t  party_foot_step
 *   int32_t  object_count
 *   [for i = 0..object_count-1:]
 *     uint8_t  type, state
 *     int32_t  x, y
 *     int32_t  level
 *     int32_t  quantity
 *     int32_t  linked_id
 *     uint32_t flags
 *   int32_t  event_count
 *   [for i = 0..event_count-1:]
 *     uint32_t type
 *     int32_t  level
 *     int32_t  x, y
 *     int32_t  arg0, arg1
 *     int32_t  fired
 *     int32_t  repeat
 *   int32_t  timer_count
 *   [for i = 0..timer_count-1:]
 *     int32_t  id
 *     uint32_t kind
 *     int32_t  level
 *     int32_t  remaining_ticks
 *     int32_t  interval_ticks
 *     uint32_t flags
 *     uint32_t userdata (= 0 on restore; caller must re-bind)
 *   uint64_t world_tick
 *   int32_t  transition_pending
 *   int32_t  transition_target
 *   int32_t  transition_x, transition_y
 *   uint64_t state_hash
 *
 * Notes:
 *   - Dungeon grid squares are NOT serialized; they are restored by
 *     re-parsing the DGN files at load time using level_loaded flags.
 *   - 3D geometry data is NOT serialized; it is restored from DGN files.
 *   - userdata pointers in timers are zeroed; caller must re-establish
 *     timer callbacks after loading.
 *
 * Source-lock reference:
 *   ReDMCSB LOADSAVE.C: F0433/F0434 (DM1 world save structure)
 *   DM Nexus (Saturn): memory card 8 KB blocks (proprietary, undocumented)
 *     — the Firestaff native format serializes the equivalent state. */

/* Magic: 'FNXW' = 'F' + ('N'<<8) + ('X'<<16) + ('W'<<24) = 0x57584E46 */
#define NEXUS_WORLD_SAVE_MAGIC   0x57584E46U
#define NEXUS_WORLD_SAVE_VERSION 1

/* Returns the number of bytes required for a serialized world.
 * Returns 0 if world is NULL. */
size_t nexus_v1_world_serialize_size(const Nexus_V1_World *world);

/* Serialize world into buf (must be >= nexus_v1_world_serialize_size()).
 * Returns bytes written, or 0 on error. */
size_t nexus_v1_world_serialize(const Nexus_V1_World *world,
                               void *buf, size_t bufsize);

/* Deserialize world from buf into world (must be allocated).
 * Returns NEXUS_SAVE_OK (0) on success, negative error code on failure.
 * On error, world contents are undefined. */
int nexus_v1_world_deserialize(Nexus_V1_World *world,
                              const void *buf, size_t bufsize);

#endif /* NEXUS_V1_WORLD_H */