#ifndef THERON_V1_WORLD_H
#define THERON_V1_WORLD_H

#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"
#include <stddef.h>
#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 3 — Core World Model
 *
 * Map loading, party placement (Theron + 3 champions), map transitions,
 * timers, object database, and deterministic world-state hashing.
 *
 * Key design constraints (from TQR provenance):
 *   - 7 mini-dungeons, 1-3 levels each (matching THQUEST.ASM).
 *   - Between-dungeon saves only (no in-dungeon save).
 *   - Theron persists fully across dungeons.
 *   - Champions reset inventories each dungeon, keep stats/skills.
 *   - 7 quest items (one per dungeon) must be found before exit.
 *   - Party of 4: Theron + 3 companions, fixed order (no party swap).
 *
 * Source references:
 *   THQUEST.ASM T000  — title/startup entry
 *   THQUEST.ASM T400  — dungeon bank loading
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  — map transitions
 *   THQUEST.ASM T700  — timers / world tick
 *   THQUEST.ASM T800  — champion persistence + inventory reset
 *   THQUEST.ASM T900  — object database / thing list
 * ══════════════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Map / level constants ─────────────────────────────────────────── */
#define THERON_MAX_LEVELS_PER_DUNGEON  3
#define THERON_MAX_MAP_SIZE            32
#define THERON_MAX_OBJECTS_PER_LEVEL  128
#define THERON_MAX_TIMERS              16
#define THERON_TICK_MS                 55  /* ~18.2 Hz world tick (matches DM1) */

/* ── Square tile types ────────────────────────────────────────────── */
#define THERON_SQUARE_WALL           0
#define THERON_SQUARE_FLOOR          1
#define THERON_SQUARE_DOOR           4   /* open/close on step/command */
#define THERON_SQUARE_PIT            2   /* trap — wounds champion */
#define THERON_SQUARE_STAIRS_UP      3   /* level N → level N-1 */
#define THERON_SQUARE_STAIRS_DOWN    13  /* level N → level N+1 */
#define THERON_SQUARE_TELEPORTER     5   /* warp to linked target */
#define THERON_SQUARE_ALARM          6   /* alert all creatures */
#define THERON_SQUARE_EXIT           8   /* quest-complete exit */
#define THERON_SQUARE_TRIGGER        9   /* event trigger */
#define THERON_SQUARE_POOL           10  /* water/food recovery */
#define THERON_SQUARE_SECRET        11  /* hidden door wall */

#define THERON_SQUARE_IS_PASSABLE(t) ((t) != THERON_SQUARE_WALL && (t) != THERON_SQUARE_SECRET)
#define THERON_SQUARE_IS_SPECIAL(t)  ((t) >= 2 && (t) != 4)

/* ── Level struct ──────────────────────────────────────────────────── */
typedef struct {
    int   level_index;          /* 0..2 within this dungeon */
    int   width, height;        /* typically 20x20 for TQR mini-dungeons */
    int   start_x, start_y;    /* party spawn position (THQUEST.ASM T520) */
    int   start_dir;            /* 0=N 1=E 2=S 3=W */
    uint8_t squares[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    int   has_3d_geometry;
    int   geometry_offset;
    int   geometry_size;
    int   thing_count;
} Theron_V1_Level;

/* ── Object database ───────────────────────────────────────────────── */

/* TQR "light" object types — subset of DM1 item field */
typedef enum {
    THERON_OBJTYPE_NONE         = 0,
    THERON_OBJTYPE_CHEST        = 1,
    THERON_OBJTYPE_DOOR         = 2,
    THERON_OBJTYPE_LEVER         = 3,
    THERON_OBJTYPE_BUTTON       = 4,
    THERON_OBJTYPE_POTION       = 5,
    THERON_OBJTYPE_SCROLL       = 6,
    THERON_OBJTYPE_FOOD         = 7,
    THERON_OBJTYPE_KEY          = 8,
    THERON_OBJTYPE_WEAPON       = 9,
    THERON_OBJTYPE_ARMOR        = 10,
    THERON_OBJTYPE_ALTAR_VI     = 11,   /* altar-of-vi resurrection point */
    THERON_OBJTYPE_TELEPORTER   = 12,   /* teleporter warp pad */
    THERON_OBJTYPE_POOL         = 13,   /* water/food recovery pool */
    THERON_OBJTYPE_ALARM        = 14,   /* alert all creatures */
    THERON_OBJTYPE_CREATURE_SPAWNER = 15, /* creature generator */
    THERON_OBJTYPE_TRIGGER      = 16,   /* event trigger */
    THERON_OBJTYPE_QUEST_ITEM   = 128,
} Theron_V1_ObjectType;

#define THERON_OBJ_F_PICKED_UP   (1U << 0)
#define THERON_OBJ_F_OPENED      (1U << 1)
#define THERON_OBJ_F_ACTIVATED   (1U << 2)
#define THERON_OBJ_F_DESTROYED   (1U << 3)
#define THERON_OBJ_F_LOCKED      (1U << 4)
#define THERON_OBJ_F_USED        (1U << 5)

typedef struct {
    int      id;
    uint8_t  type;
    uint8_t  state;       /* 0=closed/locked, 1=open, 2=used, 3=broken */
    int      x, y;
    int      level;
    int      dungeon_id;
    int      quantity;
    int      linked_id;
    uint32_t flags;
} Theron_V1_Object;

#define THERON_MAX_OBJECTS 256

/* ── Timer system ─────────────────────────────────────────────────── */
typedef enum {
    THERON_TIMER_ONESHOT   = 0,
    THERON_TIMER_REPEAT    = 1,
    THERON_TIMER_COUNTDOWN = 2,
} Theron_TimerKind;

#define THERON_TIMER_F_ACTIVE  (1U << 0)
#define THERON_TIMER_F_PAUSED (1U << 1)
#define THERON_TIMER_F_DESTROY (1U << 2)

typedef struct {
    int       id;
    Theron_TimerKind kind;
    int       level;
    int       remaining_ticks;
    int       interval_ticks;
    uint32_t  flags;
    void     *userdata;
} Theron_V1_Timer;

/* ── Map loading result codes ─────────────────────────────────────── */
typedef enum {
    THERON_MAP_OK                =  0,
    THERON_MAP_ERR_NULL          = -1,
    THERON_MAP_ERR_SIZE_TOO_SMALL = -2,
    THERON_MAP_ERR_INVALID_GRID  = -3,
    THERON_MAP_ERR_NO_ENTRANCE   = -4,
} Theron_MapLoadResult;

/* ── Transition result codes ───────────────────────────────────────── */
typedef enum {
    THERON_TRANSITION_STAIRS     = 1,
    THERON_TRANSITION_TELEPORTER = 2,
    THERON_TRANSITION_EXIT       = 3,
} Theron_TransitionType;

#define THERON_SQUARE_TO_TRANSITION_TYPE(t) \
    (((t) == THERON_SQUARE_STAIRS_UP || (t) == THERON_SQUARE_STAIRS_DOWN) \
     ? THERON_TRANSITION_STAIRS \
     : ((t) == THERON_SQUARE_TELEPORTER) \
       ? THERON_TRANSITION_TELEPORTER \
       : ((t) == THERON_SQUARE_EXIT) \
         ? THERON_TRANSITION_EXIT : 0)

/* ── World state struct ────────────────────────────────────────────── */
typedef struct {
    /* Dungeon maps — levels[7][3]: [dungeon_id-1][level_index] */
    Theron_V1_Level levels[THERON_DUNGEON_COUNT][THERON_MAX_LEVELS_PER_DUNGEON];
    int level_loaded[THERON_DUNGEON_COUNT][THERON_MAX_LEVELS_PER_DUNGEON];

    /* Dungeon progression (quest items, dungeon sequence) */
    Theron_DungeonProgression progression;

    /* Party — Theron + 3 companions */
    Theron_V1_Party party;

    /* Object pool */
    Theron_V1_Object objects[THERON_MAX_OBJECTS];
    int object_count;

    /* Timer pool */
    Theron_V1_Timer timers[THERON_MAX_TIMERS];
    int timer_count;

    /* World tick */
    uint64_t world_tick;

    /* Current dungeon/location */
    int current_dungeon;   /* 1..7 */
    int current_level;     /* 0..2 */

    /* Transition queue */
    int transition_pending;
    Theron_TransitionType transition_type;
    int transition_target_level;
    int transition_spawn_x, transition_spawn_y;

    /* Quest item tracking */
    uint8_t quest_items_in_dungeon;
    uint8_t dungeon_complete;

    /* Entry guard */
    int entry_reset_applied;

    /* Deterministic state hash */
    uint64_t state_hash;
} Theron_V1_World;

/* ── Initialization ───────────────────────────────────────────────── */
void theron_v1_world_init(Theron_V1_World *world);
void theron_v1_world_reset_for_dungeon(Theron_V1_World *world,
                                         Theron_DungeonID dungeon_id);

/* ── Map loading ─────────────────────────────────────────────────── */
/*
 * TQR 12-byte header + grid:
 *   bytes 0-1:  width  (big-endian uint16_t, LE on disk)
 *   bytes 2-3:  height (big-endian uint16_t, LE on disk)
 *   bytes 4-7:  dungeon_seed (uint32_t LE)
 *   bytes 8-9:  level_index (uint16_t LE)
 *   bytes 10-11: reserved
 *   byte 12+:  W×H grid of uint8_t tile values
 */
Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                           const uint8_t *data,
                                           int data_size,
                                           int dungeon_id,
                                           int sub_level_index);

uint8_t theron_v1_world_get_square(const Theron_V1_World *world, int x, int y);
void theron_v1_party_place(Theron_V1_World *world, int x, int y, int dir);

/* ── Object database API ─────────────────────────────────────────── */
int theron_v1_object_place(Theron_V1_World *world, Theron_V1_Object *object);
int theron_v1_object_remove(Theron_V1_World *world, int id);
Theron_V1_Object *theron_v1_object_at(Theron_V1_World *world, int level, int x, int y);
Theron_V1_Object *theron_v1_object_by_id(Theron_V1_World *world, int id);
int theron_v1_object_set_state(Theron_V1_World *world, int id, uint8_t new_state);
int theron_v1_object_set_flag(Theron_V1_World *world, int id, uint32_t flag);
int theron_v1_object_clear_flag(Theron_V1_World *world, int id, uint32_t flag);

/* ── Timer API ───────────────────────────────────────────────────── */
int  theron_v1_timer_add(Theron_V1_World *world,
                         Theron_TimerKind kind,
                         int level,
                         int remaining_ticks,
                         int interval_ticks,
                         void *userdata);
void theron_v1_timer_remove(Theron_V1_World *world, int id);
void theron_v1_timer_pause(Theron_V1_World *world, int id);
void theron_v1_timer_resume(Theron_V1_World *world, int id);
void theron_v1_tick_timers(Theron_V1_World *world);
void theron_v1_timers_clear_level(Theron_V1_World *world, int level);

/* ── Level transitions ────────────────────────────────────────────── */
Theron_TransitionType theron_v1_check_transition(Theron_V1_World *world, int x, int y);
int theron_v1_transition_execute(Theron_V1_World *world);

/* ── World tick ───────────────────────────────────────────────────── */
void theron_v1_world_tick(Theron_V1_World *world);

/* ── Deterministic world hash (FNV-1a 64-bit) ─────────────────────── */
#define THERON_HASH_SEED_PARTY   0x50415254UL  /* 'PART' */
#define THERON_HASH_SEED_OBJECT 0x4F424A45UL  /* 'OBJE' */
#define THERON_HASH_SEED_TIMER  0x54494D45UL  /* 'TIME' */
#define THERON_HASH_SEED_DUNG   0x444E4753UL  /* 'DNGS' */
#define THERON_HASH_FNV_OFFSET  0xCBF29CE484222325UL
#define THERON_HASH_FNV_PRIME   0x00000100000001B3UL

uint64_t theron_v1_world_hash(const Theron_V1_World *world);
void     theron_v1_world_hash_inject(Theron_V1_World *world, uint64_t seed);

/* ── Quest item helpers ─────────────────────────────────────────────── */
int   theron_v1_check_quest_item(const Theron_V1_World *world);
uint8_t theron_v1_collect_quest_item(Theron_V1_World *world, uint8_t item_bit);

/* ── Binary serialization ─────────────────────────────────────────── */
#define THERON_WORLD_SAVE_MAGIC   0x574E5254U  /* 'TRNW' */
#define THERON_WORLD_SAVE_VERSION 1

size_t theron_v1_world_serialize_size(const Theron_V1_World *world);
size_t theron_v1_world_serialize(const Theron_V1_World *world,
                                  void *buf, size_t bufsize);
int    theron_v1_world_deserialize(Theron_V1_World *world,
                                    const void *buf, size_t bufsize);

/* ── Source citation ───────────────────────────────────────────────── */
const char *theron_v1_world_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_WORLD_H */
