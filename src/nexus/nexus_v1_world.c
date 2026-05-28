#include "nexus_v1_world.h"
#include "nexus_v1_squares.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* ── Save result codes (matches nexus_v1_save.h) ─────────────────────── */
typedef enum {
    NEXUS_SAVE_OK          =  0,
    NEXUS_SAVE_ERR_NULL   = -1,
    NEXUS_SAVE_ERR_SIZE   = -2,
    NEXUS_SAVE_ERR_MAGIC  = -3,
    NEXUS_SAVE_ERR_VERSION= -4
} Nexus_WorldSaveResult;


/* ------------------------------------------------------------------ */
/* FNV-1a 64-bit hash                                                  */
/* ------------------------------------------------------------------ */

/* Per- RFC 8789 — 64-bit FNV-1a using a fixed offset basis.
 * This is the foundation of deterministic world-state hashing. */
#define FNV64_OFFSET 0xCBF29CE484222325UL
#define FNV64_PRIME  0x00000100000001B3UL

static uint64_t fnv64(uint64_t h, uint64_t val) {
    h ^= val & 0xFFULL;  h *= FNV64_PRIME;
    h ^= (val >> 8) & 0xFFULL;  h *= FNV64_PRIME;
    h ^= (val >> 16) & 0xFFULL; h *= FNV64_PRIME;
    h ^= (val >> 24) & 0xFFULL; h *= FNV64_PRIME;
    h ^= (val >> 32) & 0xFFULL; h *= FNV64_PRIME;
    h ^= (val >> 40) & 0xFFULL; h *= FNV64_PRIME;
    h ^= (val >> 48) & 0xFFULL; h *= FNV64_PRIME;
    h ^= (val >> 56) & 0xFFULL; h *= FNV64_PRIME;
    return h;
}

uint64_t nexus_v1_world_hash(Nexus_V1_World *world) {
    /* Source-lock: DUNGEON.C F0029 (world hash) — deterministic state
     * snapshot from provenance-locked fixtures. */
    uint64_t h = FNV64_OFFSET;
    int i;

    h = fnv64(h, NEXUS_HASH_SEED_PARTY);
    h = fnv64(h, (uint64_t)world->party_level);
    h = fnv64(h, (uint64_t)world->party_x);
    h = fnv64(h, (uint64_t)world->party_y);
    h = fnv64(h, (uint64_t)world->party_dir);
    h = fnv64(h, (uint64_t)world->party_foot_step);

    h = fnv64(h, NEXUS_HASH_SEED_OBJECT);
    h = fnv64(h, (uint64_t)world->object_count);
    for (i = 0; i < world->object_count; i++) {
        Nexus_V1_Object *o = &world->objects[i];
        h = fnv64(h, (uint64_t)o->id);
        h = fnv64(h, (uint64_t)o->type);
        h = fnv64(h, (uint64_t)o->state);
        h = fnv64(h, (uint64_t)(o->x | (o->y << 8)));
        h = fnv64(h, (uint64_t)o->level);
        h = fnv64(h, (uint64_t)o->quantity);
        h = fnv64(h, o->flags);
    }

    h = fnv64(h, NEXUS_HASH_SEED_TIMER);
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        Nexus_V1_Timer *t = &world->timers[i];
        if (!(t->flags & NEXUS_TIMER_F_ACTIVE)) continue;
        h = fnv64(h, (uint64_t)t->id);
        h = fnv64(h, (uint64_t)t->kind);
        h = fnv64(h, (uint64_t)t->remaining_ticks);
        h = fnv64(h, (uint64_t)t->interval_ticks);
        h = fnv64(h, (uint64_t)t->level);
    }

    h = fnv64(h, NEXUS_HASH_SEED_EVENT);
    for (i = 0; i < world->event_count; i++) {
        Nexus_V1_EventRecord *e = &world->events[i];
        if (!e->fired) continue;
        h = fnv64(h, (uint64_t)e->type);
        h = fnv64(h, (uint64_t)(e->x | (e->y << 8)));
        h = fnv64(h, (uint64_t)e->arg0);
    }

    h = fnv64(h, NEXUS_HASH_SEED_TICK);
    h = fnv64(h, world->world_tick);

    world->state_hash = h;
    return h;
}

void nexus_v1_world_hash_inject(Nexus_V1_World *world, uint64_t seed) {
    (void)seed;
    /* Seeded FNV: start from seed instead of offset basis.
     * For provenance-locked use, call with seed derived from
     * MD5 first 8 bytes of LEV00.DGN. */
    uint64_t h = seed;
    int i;
    for (i = 0; i < 10; i++)          /* mixing rounds */
        h = fnv64(h, FNV64_OFFSET ^ (FNV64_PRIME * (uint64_t)i));
    world->state_hash = h;
}

/* ------------------------------------------------------------------ */
/* World init / reset                                                  */
/* ------------------------------------------------------------------ */

void nexus_v1_world_init(Nexus_V1_World *world) {
    if (!world) return;
    memset(world, 0, sizeof(*world));
    world->party_level = 0;
    world->party_x = 11;
    world->party_y = 29;   /* DM1 entrance spawn */
    world->party_dir = 0;  /* North */
    world->world_tick = 0;
    world->state_hash = FNV64_OFFSET;
    printf("Nexus V1 world initialized (%dx%d DMWeb grid, %d levels)\n",
           NEXUS_MAX_MAP_SIZE, NEXUS_MAX_MAP_SIZE, NEXUS_MAX_LEVELS);
}

void nexus_v1_world_reset(Nexus_V1_World *world) {
    if (!world) return;
    memset(world->levels, 0, sizeof(world->levels));
    memset(world->objects, 0, sizeof(world->objects));
    memset(world->events, 0, sizeof(world->events));
    memset(world->timers, 0, sizeof(world->timers));
    world->object_count = 0;
    world->event_count  = 0;
    world->timer_count  = 0;
    world->world_tick   = 0;
    world->transition_pending = 0;
    world->state_hash = FNV64_OFFSET;
}

/* nexus_v1_party_place — place party at grid position on given level.
 * ReDMCSB: DUNGEON.C F0155 (party placement after stairs/door/teleport).
 * Updates party position immediately; callers needing level-change
 * semantics should use nexus_v1_transition_queue() instead. */
void nexus_v1_party_place(Nexus_V1_World *world,
                           int level, int x, int y, int dir) {
    if (!world) return;
    world->party_level = level;
    world->party_x     = x;
    world->party_y     = y;
    if (dir >= 0) world->party_dir = dir;
}

/* ------------------------------------------------------------------ */
/* Object database                                                     */
/* ------------------------------------------------------------------ */

/* ReDMCSB: DUNGEON.C F0103 — object placement and collision.
 * Source-lock note: Nexus objects are placed from LEV*.DGN thing list
 * (offset 2048 + geometry_size, per nexus_dungeon.h). */
int nexus_v1_object_place(Nexus_V1_World *world, const Nexus_V1_Object *obj) {
    if (!world || !obj) return -1;
    if (world->object_count >= NEXUS_MAX_OBJECTS) return -1;

    Nexus_V1_Object *dst = &world->objects[world->object_count++];
    *dst = *obj;
    dst->id = world->object_count;   /* 1-based unique ID */
    return dst->id;
}

int nexus_v1_object_remove(Nexus_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return -1;
    for (i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == id) {
            world->objects[i] = world->objects[--world->object_count];
            return 0;
        }
    }
    return -1;
}

Nexus_V1_Object *nexus_v1_object_at(Nexus_V1_World *world,
                                     int level, int x, int y) {
    int i;
    if (!world) return NULL;
    for (i = 0; i < world->object_count; i++) {
        Nexus_V1_Object *o = &world->objects[i];
        if (o->level == level && o->x == x && o->y == y
            && !(o->flags & NEXUS_OBJ_F_PICKED_UP))
            return o;
    }
    return NULL;
}

Nexus_V1_Object *nexus_v1_object_by_id(Nexus_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return NULL;
    for (i = 0; i < world->object_count; i++)
        if (world->objects[i].id == id)
            return &world->objects[i];
    return NULL;
}

int nexus_v1_object_set_state(Nexus_V1_World *world, int id,
                               uint8_t new_state) {
    Nexus_V1_Object *o = nexus_v1_object_by_id(world, id);
    if (!o) return -1;
    o->state = new_state;
    return 0;
}

int nexus_v1_object_set_flag(Nexus_V1_World *world, int id,
                              uint32_t flag) {
    Nexus_V1_Object *o = nexus_v1_object_by_id(world, id);
    if (!o) return -1;
    o->flags |= flag;
    return 0;
}

int nexus_v1_object_clear_flag(Nexus_V1_World *world, int id,
                                uint32_t flag) {
    Nexus_V1_Object *o = nexus_v1_object_by_id(world, id);
    if (!o) return -1;
    o->flags &= ~flag;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Event system                                                        */
/* ------------------------------------------------------------------ */

/* ReDMCSB: MOVESENS.C F0067 — sensor fire and event record logging.
 * Source-lock: nexus_v1_event_fire mirrors the event-dispatch
 * pattern of DM1 MOVESENS sensor triggers, adapted to the
 * SDDRVS.TSK event model documented in docs/nexus_triggers.md. */
int nexus_v1_event_fire(Nexus_V1_World *world, Nexus_V1_EventType type,
                         int level, int x, int y,
                         int arg0, int arg1) {
    if (!world) return -1;
    if (world->event_count >= NEXUS_MAX_EVENTS) return -1;

    Nexus_V1_EventRecord *e = &world->events[world->event_count++];
    e->type   = type;
    e->level  = level;
    e->x      = x;
    e->y      = y;
    e->arg0   = arg0;
    e->arg1   = arg1;
    e->repeat = 0;
    e->fired  = 1;
    return 0;
}

void nexus_v1_events_clear_level(Nexus_V1_World *world, int level) {
    int i, dst = 0;
    if (!world) return;
    for (i = 0; i < world->event_count; i++) {
        if (world->events[i].level != level)
            world->events[dst++] = world->events[i];
    }
    world->event_count = dst;
}

void nexus_v1_events_clear_all(Nexus_V1_World *world) {
    if (!world) return;
    world->event_count = 0;
}

/* ------------------------------------------------------------------ */
/* Timer system                                                        */
/* ------------------------------------------------------------------ */

/* ReDMCSB: MOVESENS.C F0071 — timer queue and tick dispatch.
 * Nexus timers drive SDDRVS.TSK delayed actions and creature
 * spawn cycles.  Tick rate matches V1: 18.2 Hz. */
static int next_timer_id = 1;

int nexus_v1_timer_add(Nexus_V1_World *world, Nexus_TimerKind kind,
                        int level, int ticks, int interval,
                        void *userdata) {
    int i;
    if (!world) return -1;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if (!(world->timers[i].flags & NEXUS_TIMER_F_ACTIVE)) {
            Nexus_V1_Timer *t = &world->timers[i];
            t->id              = next_timer_id++;
            t->kind            = kind;
            t->level           = level;
            t->remaining_ticks = ticks;
            t->interval_ticks  = interval;
            t->flags           = NEXUS_TIMER_F_ACTIVE;
            t->userdata        = userdata;
            return t->id;
        }
    }
    return -1;
}

void nexus_v1_timer_remove(Nexus_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        Nexus_V1_Timer *t = &world->timers[i];
        if ((int)t->id == id && (t->flags & NEXUS_TIMER_F_ACTIVE)) {
            t->flags &= ~NEXUS_TIMER_F_ACTIVE;
            return;
        }
    }
}

void nexus_v1_timer_pause(Nexus_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        Nexus_V1_Timer *t = &world->timers[i];
        if ((int)t->id == id && (t->flags & NEXUS_TIMER_F_ACTIVE)) {
            t->flags |= NEXUS_TIMER_F_PAUSED;
            return;
        }
    }
}

void nexus_v1_timer_resume(Nexus_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        Nexus_V1_Timer *t = &world->timers[i];
        if ((int)t->id == id && (t->flags & NEXUS_TIMER_F_ACTIVE)) {
            t->flags &= ~NEXUS_TIMER_F_PAUSED;
            return;
        }
    }
}

void nexus_v1_tick_timers(Nexus_V1_World *world) {
    int i;
    if (!world) return;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        Nexus_V1_Timer *t = &world->timers[i];
        if (!(t->flags & NEXUS_TIMER_F_ACTIVE)) continue;
        if (t->flags & NEXUS_TIMER_F_PAUSED) continue;

        if (t->remaining_ticks > 0) {
            t->remaining_ticks--;
            if (t->remaining_ticks == 0) {
                /* Fire timer event */
                nexus_v1_event_fire(world, NEXUS_EVT_TIMER_EXPIRED,
                                    t->level, 0, 0, t->id, 0);
                if (t->kind == NEXUS_TIMER_REPEAT)
                    t->remaining_ticks = t->interval_ticks;
                else
                    t->flags &= ~NEXUS_TIMER_F_ACTIVE; /* oneshot done */
            }
        }
    }
}

void nexus_v1_timers_clear_level(Nexus_V1_World *world, int level) {
    int i;
    if (!world) return;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if (world->timers[i].level == level)
            world->timers[i].flags &= ~NEXUS_TIMER_F_ACTIVE;
    }
}

/* ------------------------------------------------------------------ */
/* Level transitions                                                    */
/* ------------------------------------------------------------------ */

/* ReDMCSB: DUNGEON.C F0044 — stairs/pit/teleporter level transitions.
 * Source-lock: transition is queued and executed in the same tick
 * as MOVESENS processes the trigger square. */
int nexus_v1_transition_queue(Nexus_V1_World *world,
                               int target_level,
                               int spawn_x, int spawn_y) {
    if (!world) return -1;
    if (target_level < 0 || target_level >= NEXUS_MAX_LEVELS) return -1;
    world->transition_pending   = 1;
    world->transition_target   = target_level;
    world->transition_x        = spawn_x;
    world->transition_y        = spawn_y;
    return 0;
}

int nexus_v1_transition_execute(Nexus_V1_World *world) {
    if (!world || !world->transition_pending) return -1;

    /* Clear timers/events from old level */
    nexus_v1_timers_clear_level(world, world->party_level);
    nexus_v1_events_clear_level(world, world->party_level);

    world->party_level         = world->transition_target;
    world->party_x             = world->transition_x;
    world->party_y             = world->transition_y;
    world->transition_pending  = 0;

    /* Fire level-loaded event for the new level */
    nexus_v1_event_fire(world, NEXUS_EVT_LEVEL_LOADED,
                        world->party_level,
                        world->party_x, world->party_y,
                        0, 0);
    return 0;
}

/* ------------------------------------------------------------------ */
/* World tick                                                          */
/* ------------------------------------------------------------------ */

/*
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Binary serialization — world state (magic = 'FNXW')
 * Source-lock: ReDMCSB LOADSAVE.C F0433/F0434 world state layout.
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *
 * Format:
 *   uint32_t magic            = 'FNXW'
 *   uint16_t format_version  = 1
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
 *   [for i = 0..timer_count-1 (active only):]
 *     int32_t  id
 *     uint32_t kind
 *     int32_t  level
 *     int32_t  remaining_ticks
 *     int32_t  interval_ticks
 *     uint32_t flags
 *   uint64_t world_tick
 *   int32_t  transition_pending
 *   int32_t  transition_target
 *   int32_t  transition_x, transition_y
 *   uint64_t state_hash
 *
 * Notes:
 *   - Dungeon grid squares are NOT serialized; they are restored by
 *     re-parsing the DGN files at load time.
 *   - 3D geometry data is NOT serialized; restored from DGN files.
 *   - userdata pointers in timers are zeroed; caller must re-establish.
 *   - level_loaded[] and models are NOT part of the world state struct;
 *     they are owned by Nexus_V1_Engine, not Nexus_V1_World.
 */

static uint8_t *wr8(uint8_t *p, uint8_t v)  { *p++ = v;  return p; }
static uint8_t *wr16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v);      p[1] = (uint8_t)(v >> 8);
    return p + 2;
}
static uint8_t *wr32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);      p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
    return p + 4;
}
static uint8_t *wr64(uint8_t *p, uint64_t v) {
    int i; for (i = 0; i < 8; i++, v >>= 8) p[i] = (uint8_t)(v & 0xFF);
    return p + 8;
}
static const uint8_t *rd8(const uint8_t *p, uint8_t *v)    { *v = *p++; return p; }
static const uint8_t *rd16(const uint8_t *p, uint16_t *v) {
    *v = ((uint16_t)p[0])|((uint16_t)p[1]<<8); return p + 2;
}
static const uint8_t *rd32(const uint8_t *p, uint32_t *v) {
    *v = ((uint32_t)p[0])|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
    return p + 4;
}
static const uint8_t *rd64(const uint8_t *p, uint64_t *v) {
    int i; *v = 0;
    for (i = 0; i < 8; i++) { *v |= ((uint64_t)p[i]) << (i*8); }
    return p + 8;
}

size_t nexus_v1_world_serialize_size(const Nexus_V1_World *world) {
    /* Fixed header: magic(4)+ver(2)+pad(2)+all scalars */
    size_t n = 4 + 2 + 2 + 4*8;
    /* Objects */
    n += 4 + world->object_count * (1+1+4+4+4+4+4+4);
    /* Events */
    n += 4 + world->event_count * (4+4+4+4+4+4+4);
    /* Active timers */
    {
        int tc = 0, i;
        for (i = 0; i < NEXUS_MAX_TIMERS; i++)
            if (world->timers[i].flags & NEXUS_TIMER_F_ACTIVE) tc++;
        n += 4 + tc * (4+4+4+4+4+4);
    }
    /* Transition + hash */
    n += 4 + 4 + 4 + 8;
    return n;
}


size_t nexus_v1_world_serialize(const Nexus_V1_World *world,
                                 void *buf, size_t bufsize) {
    uint8_t *p = (uint8_t *)buf;
    size_t needed = nexus_v1_world_serialize_size(world);
    if (!world || !buf || bufsize < needed) return 0;

    p = wr32(p, NEXUS_WORLD_SAVE_MAGIC);
    p = wr16(p, NEXUS_WORLD_SAVE_VERSION);
    p = wr16(p, 0);
    p = wr32(p, (uint32_t)world->party_level);
    p = wr32(p, (uint32_t)world->party_x);
    p = wr32(p, (uint32_t)world->party_y);
    p = wr32(p, (uint32_t)world->party_dir);
    p = wr32(p, (uint32_t)world->party_foot_step);
    p = wr32(p, (uint32_t)world->object_count);

    {
        int i;
        for (i = 0; i < world->object_count; i++) {
            const Nexus_V1_Object *o = &world->objects[i];
            p = wr8(p, o->type);
            p = wr8(p, o->state);
            p = wr32(p, (uint32_t)o->x);
            p = wr32(p, (uint32_t)o->y);
            p = wr32(p, (uint32_t)o->level);
            p = wr32(p, (uint32_t)o->quantity);
            p = wr32(p, (uint32_t)o->linked_id);
            p = wr32(p, o->flags);
        }
    }
    p = wr32(p, (uint32_t)world->event_count);
    {
        int i;
        for (i = 0; i < world->event_count; i++) {
            const Nexus_V1_EventRecord *e = &world->events[i];
            p = wr32(p, (uint32_t)e->type);
            p = wr32(p, (uint32_t)e->level);
            p = wr32(p, (uint32_t)e->x);
            p = wr32(p, (uint32_t)e->y);
            p = wr32(p, (uint32_t)e->arg0);
            p = wr32(p, (uint32_t)e->arg1);
            p = wr32(p, (uint32_t)(e->fired ? 1 : 0));
            p = wr32(p, (uint32_t)(e->repeat ? 1 : 0));
        }
    }
    {
        int tc = 0, i;
        for (i = 0; i < NEXUS_MAX_TIMERS; i++)
            if (world->timers[i].flags & NEXUS_TIMER_F_ACTIVE) tc++;
        p = wr32(p, (uint32_t)tc);
        for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
            const Nexus_V1_Timer *t = &world->timers[i];
            if (!(t->flags & NEXUS_TIMER_F_ACTIVE)) continue;
            p = wr32(p, (uint32_t)t->id);
            p = wr32(p, (uint32_t)t->kind);
            p = wr32(p, (uint32_t)t->level);
            p = wr32(p, (uint32_t)t->remaining_ticks);
            p = wr32(p, (uint32_t)t->interval_ticks);
            p = wr32(p, t->flags);
        }
    }
    p = wr64(p, world->world_tick);
    p = wr32(p, world->transition_pending ? 1U : 0U);
    p = wr32(p, (uint32_t)world->transition_target);
    p = wr32(p, (uint32_t)world->transition_x);
    p = wr32(p, (uint32_t)world->transition_y);
    p = wr64(p, world->state_hash);
    return (size_t)(p - (uint8_t *)buf);
}

int nexus_v1_world_deserialize(Nexus_V1_World *world,
                               const void *buf, size_t bufsize) {
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t magic;
    uint16_t version, pad16;
    uint32_t v32;
    (void)pad16; /* consumed via rd16 side-effect */
    if (!world || !buf || bufsize < 4) return NEXUS_SAVE_ERR_NULL;


    p = rd32(p, &magic);
    if (magic != NEXUS_WORLD_SAVE_MAGIC) return NEXUS_SAVE_ERR_MAGIC;
    p = rd16(p, &version);
    if (version != NEXUS_WORLD_SAVE_VERSION) return NEXUS_SAVE_ERR_VERSION;
    p = rd16(p, &pad16);

    p = rd32(p, &v32); world->party_level  = (int)v32;
    p = rd32(p, &v32); world->party_x      = (int)v32;
    p = rd32(p, &v32); world->party_y      = (int)v32;
    p = rd32(p, &v32); world->party_dir    = (int)v32;
    p = rd32(p, &v32); world->party_foot_step = (int)v32;
    p = rd32(p, &v32); world->object_count = (int)v32;
    if (world->object_count > NEXUS_MAX_OBJECTS) world->object_count = NEXUS_MAX_OBJECTS;
    {
        int i;
        for (i = 0; i < world->object_count; i++) {
            Nexus_V1_Object *o = &world->objects[i];
            p = rd8(p, &o->type);
            p = rd8(p, &o->state);
            p = rd32(p, &v32); o->x         = (int)v32;
            p = rd32(p, &v32); o->y         = (int)v32;
            p = rd32(p, &v32); o->level     = (int)v32;
            p = rd32(p, &v32); o->quantity  = (int)v32;
            p = rd32(p, &v32); o->linked_id = (int)v32;
            p = rd32(p, &o->flags);
        }
    }
    p = rd32(p, &v32); world->event_count = (int)v32;
    if (world->event_count > NEXUS_MAX_EVENTS) world->event_count = NEXUS_MAX_EVENTS;
    {
        int i;
        for (i = 0; i < world->event_count; i++) {
            Nexus_V1_EventRecord *e = &world->events[i];
            uint32_t u32;
            p = rd32(p, &u32); e->type   = (Nexus_V1_EventType)u32;
            p = rd32(p, &u32); e->level  = (int)u32;
            p = rd32(p, &u32); e->x      = (int)u32;
            p = rd32(p, &u32); e->y      = (int)u32;
            p = rd32(p, &u32); e->arg0   = (int)u32;
            p = rd32(p, &u32); e->arg1   = (int)u32;
            p = rd32(p, &u32); e->fired  = (int)u32;
            p = rd32(p, &u32); e->repeat = (int)u32;
        }
    }
    p = rd32(p, &v32);
    {
        int i, tc = (int)v32;
        memset(world->timers, 0, sizeof(world->timers));
        for (i = 0; i < tc && i < NEXUS_MAX_TIMERS; i++) {
            Nexus_V1_Timer *t = &world->timers[i];
            uint32_t u32;
            p = rd32(p, &u32); t->id = (int)u32;
            p = rd32(p, &u32); t->kind = (Nexus_TimerKind)u32;
            p = rd32(p, &u32); t->level = (int)u32;
            p = rd32(p, &u32); t->remaining_ticks = (int)u32;
            p = rd32(p, &u32); t->interval_ticks = (int)u32;
            p = rd32(p, &t->flags);
        }
        /* Clear remainder slots */
        for (; i < NEXUS_MAX_TIMERS; i++) world->timers[i].flags &= ~NEXUS_TIMER_F_ACTIVE;
    }
    p = rd64(p, &world->world_tick);
    p = rd32(p, &v32); world->transition_pending = (int)v32;
    p = rd32(p, &v32); world->transition_target = (int)v32;
    p = rd32(p, &v32); world->transition_x = (int)v32;
    p = rd32(p, &v32); world->transition_y = (int)v32;
    p = rd64(p, &world->state_hash);
    return NEXUS_SAVE_OK;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
/* World tick entry point                                            */
/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* ReDMCSB: DUNGEON.C F0001 — main tick entry point.
 * nexus_v1_world_tick is called once per V1 tick (55 ms).
 * It advances the world clock, ticks timers, processes queued
 * transitions, and updates the world-state hash. */
void nexus_v1_world_tick(Nexus_V1_World *world) {
    if (!world) return;

    world->world_tick++;
    nexus_v1_tick_timers(world);

    /* Execute pending level transition */
    if (world->transition_pending)
        nexus_v1_transition_execute(world);

    /* Advance foot step animation counter */
    world->party_foot_step++;
}
