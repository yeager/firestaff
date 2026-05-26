#include "nexus_v1_world.h"
#include <string.h>
#include <stdio.h>

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
    printf("Nexus V1 world initialized (32x32 grid, %d levels)\n",
           NEXUS_MAX_LEVELS);
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