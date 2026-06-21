/*
 * dm2_v1_timeline.c - DM2 V1 Timeline Wiring Implementation
 *
 * Phase 4 (mechanics parity) source-lock.
 *
 * The DM2 timeline is a time-ordered event queue with these properties:
 *   - Events are scheduled at absolute times (fire_at_ms).
 *   - A tick walker fires events whose fire_at_ms <= now_ms.
 *   - Past events are fired immediately; future events are held.
 *   - Event firing is idempotent (each event fires once unless rescheduled).
 *
 * This module provides:
 *   1. Built-in timeline catalog (6 events: NPC_MOVE + CREATURE_SPAWN +
 *      DOOR_LOCK + DOOR_UNLOCK + MESSAGE_DISPLAY).
 *   2. Schedule API (dm2_v1_timeline_schedule).
 *   3. Tick walker (dm2_v1_timeline_tick) that fires due events.
 *   4. Explicit fire / cancel for tests.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_timeline.cpp        - timeline core
 *   skproject/SKWIN/DME.h:1780-1850          - timeline_event_t
 *   skproject/SKWIN/SkGlobal.cpp:1280-1350   - dTimelineTable
 *   ReDMCSB TIMELINE.C:43-220                - F0256/F0261 timeline events
 *   ReDMCSB GAMELOOP.C:69                    - F0261_TIMELINE_Process_CPSEF
 *   ReDMCSB MOVESENS.C:1000-1100             - F0268_SENSOR_AddEvent
 *   ReDMCSB TIMELINE.C:882-908               - door destruction event (C02)
 *
 * DM2 difference vs DM1:
 *   - DM1 timeline (TIMELINE.C) has 60+ event types (C01..C70).
 *   - DM2 uses the same machinery but with a smaller built-in set
 *     (NPC move + creature spawn + door lock/unlock + message).
 *
 * V1 invariant: timeline events mutate world state but NEVER party
 * state (HP/mana/food/water/direction/position).
 */

#include "dm2_v1_timeline.h"

#include <string.h>

/* ── Built-in timeline catalog (6 events) ─────────────────────────── */
static const DM2_V1_TimelineEvent g_builtin_events[DM2_TIMELINE_NUM_BUILTIN] = {
    /* 1: NPC_MOVE at t=0 (immediate, NPC walks to throne) */
    {
        1, DM2_TIMELINE_EVENT_NPC_MOVE, 0,
        8, 8, 0, 0, 0, NULL
    },
    /* 2: CREATURE_SPAWN at t=5000 (5s after start) */
    {
        2, DM2_TIMELINE_EVENT_CREATURE_SPAWN, 5000,
        10, 10, 0, 1, 0, NULL
    },
    /* 3: DOOR_LOCK at t=10000 (10s, lock the throne room door) */
    {
        3, DM2_TIMELINE_EVENT_DOOR_LOCK, 10000,
        8, 9, 0, 0, 1, NULL
    },
    /* 4: DOOR_UNLOCK at t=30000 (30s, unlock after combat) */
    {
        4, DM2_TIMELINE_EVENT_DOOR_UNLOCK, 30000,
        8, 9, 0, 0, 1, NULL
    },
    /* 5: MESSAGE_DISPLAY at t=2000 (2s after start) */
    {
        5, DM2_TIMELINE_EVENT_MESSAGE_DISPLAY, 2000,
        0, 0, 0, 0, 0, "The dungeon awakens..."
    },
    /* 6: MESSAGE_DISPLAY at t=15000 (periodic reminder) */
    {
        6, DM2_TIMELINE_EVENT_MESSAGE_DISPLAY, 15000,
        0, 0, 0, 0, 0, "You feel a chill down your spine."
    },
};

/* ── Module state ─────────────────────────────────────────────────── */
typedef struct {
    int now_ms;
    int queue_count;
    int event_id_at_slot[DM2_TIMELINE_MAX_EVENTS];  /* event_id or DM2_TIMELINE_NONE */
    DM2_V1_TimelineEventState states[DM2_TIMELINE_MAX_EVENTS];
    DM2_V1_TimelineEvent queue[DM2_TIMELINE_MAX_EVENTS];  /* scheduled event copies */
    int total_fires;
    int total_ticks;
    int initialized;
} DM2_V1_TimelineRuntime;

static DM2_V1_TimelineRuntime s_runtime;

static void ensure_init(void) {
    if (s_runtime.initialized) return;
    memset(&s_runtime, 0, sizeof(s_runtime));
    s_runtime.now_ms = 0;
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        s_runtime.event_id_at_slot[i] = DM2_TIMELINE_NONE;
    }
    s_runtime.initialized = 1;
}

/* ── Internal: find slot for event_id, or first free slot ───────── */
static int find_slot_for(int event_id) {
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        if (s_runtime.event_id_at_slot[i] == event_id) return i;
    }
    return -1;
}

static int find_free_slot(void) {
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        if (s_runtime.event_id_at_slot[i] == DM2_TIMELINE_NONE) return i;
    }
    return -1;
}

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_timeline_reset_state(void) {
    memset(&s_runtime, 0, sizeof(s_runtime));
    s_runtime.now_ms = 0;
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        s_runtime.event_id_at_slot[i] = DM2_TIMELINE_NONE;
    }
    s_runtime.initialized = 1;
}

void dm2_v1_timeline_set_now_ms(int now_ms) {
    ensure_init();
    s_runtime.now_ms = now_ms;
}

int dm2_v1_timeline_get_now_ms(void) {
    ensure_init();
    return s_runtime.now_ms;
}

/* ── Catalog ────────────────────────────────────────────────────── */
int dm2_v1_timeline_get_builtin_count(void) {
    return DM2_TIMELINE_NUM_BUILTIN;
}

const DM2_V1_TimelineEvent *dm2_v1_timeline_get_builtin(int event_id) {
    for (int i = 0; i < DM2_TIMELINE_NUM_BUILTIN; i++) {
        if (g_builtin_events[i].event_id == event_id) {
            return &g_builtin_events[i];
        }
    }
    return NULL;
}

int dm2_v1_timeline_lookup_index(int event_id) {
    for (int i = 0; i < DM2_TIMELINE_NUM_BUILTIN; i++) {
        if (g_builtin_events[i].event_id == event_id) return i;
    }
    return -1;
}

/* ── Init: copy built-in events into queue ──────────────────────── */
int dm2_v1_timeline_init(void) {
    ensure_init();
    s_runtime.queue_count = 0;
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        s_runtime.event_id_at_slot[i] = DM2_TIMELINE_NONE;
        s_runtime.states[i].event_id = 0;
        s_runtime.states[i].active = 0;
        s_runtime.states[i].fired_count = 0;
        s_runtime.states[i].fire_at_ms = 0;
    }
    for (int i = 0; i < DM2_TIMELINE_NUM_BUILTIN; i++) {
        int slot = find_free_slot();
        if (slot < 0) return (int)DM2_TIMELINE_RESULT_QUEUE_FULL;
        s_runtime.queue[slot] = g_builtin_events[i];
        s_runtime.event_id_at_slot[slot] = g_builtin_events[i].event_id;
        s_runtime.states[slot].event_id = g_builtin_events[i].event_id;
        s_runtime.states[slot].fire_at_ms = g_builtin_events[i].fire_at_ms;
        s_runtime.queue_count++;
    }
    return (int)DM2_TIMELINE_RESULT_OK;
}

/* ── Schedule ───────────────────────────────────────────────────── */
int dm2_v1_timeline_schedule(int event_id, int fire_at_ms) {
    ensure_init();
    if (fire_at_ms < 0) return (int)DM2_TIMELINE_RESULT_INVALID_TIME;
    /* If event is already in queue, update its fire_at_ms. */
    int slot = find_slot_for(event_id);
    if (slot >= 0) {
        s_runtime.queue[slot].fire_at_ms = fire_at_ms;
        s_runtime.states[slot].fire_at_ms = fire_at_ms;
        s_runtime.states[slot].fired_count = 0;  /* re-arm */
        s_runtime.states[slot].active = 0;
        return (int)DM2_TIMELINE_RESULT_OK;
    }
    /* Otherwise add a new event from the catalog. */
    const DM2_V1_TimelineEvent *builtin = dm2_v1_timeline_get_builtin(event_id);
    if (!builtin) return (int)DM2_TIMELINE_RESULT_NOT_FOUND;
    slot = find_free_slot();
    if (slot < 0) return (int)DM2_TIMELINE_RESULT_QUEUE_FULL;
    s_runtime.queue[slot] = *builtin;
    s_runtime.queue[slot].fire_at_ms = fire_at_ms;
    s_runtime.event_id_at_slot[slot] = event_id;
    s_runtime.states[slot].event_id = event_id;
    s_runtime.states[slot].fire_at_ms = fire_at_ms;
    s_runtime.states[slot].fired_count = 0;
    s_runtime.states[slot].active = 0;
    s_runtime.queue_count++;
    return (int)DM2_TIMELINE_RESULT_OK;
}

/* ── Tick ───────────────────────────────────────────────────────── */
int dm2_v1_timeline_tick(int now_ms) {
    ensure_init();
    s_runtime.now_ms = now_ms;
    s_runtime.total_ticks++;
    int fired = 0;
    /* Sort events by fire_at_ms so earliest fires first. */
    for (int i = 0; i < DM2_TIMELINE_MAX_EVENTS; i++) {
        if (s_runtime.event_id_at_slot[i] == DM2_TIMELINE_NONE) continue;
        if (s_runtime.states[i].fired_count > 0) continue;
        if (s_runtime.queue[i].fire_at_ms <= now_ms) {
            s_runtime.states[i].active = 1;
            s_runtime.states[i].fired_count++;
            s_runtime.total_fires++;
            fired++;
        }
    }
    return fired;
}

/* ── Explicit fire ──────────────────────────────────────────────── */
int dm2_v1_timeline_fire(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return (int)DM2_TIMELINE_RESULT_NOT_FOUND;
    s_runtime.states[slot].active = 1;
    s_runtime.states[slot].fired_count++;
    s_runtime.total_fires++;
    return (int)DM2_TIMELINE_RESULT_OK;
}

/* ── Cancel ─────────────────────────────────────────────────────── */
int dm2_v1_timeline_cancel(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return (int)DM2_TIMELINE_RESULT_NOT_FOUND;
    s_runtime.event_id_at_slot[slot] = DM2_TIMELINE_NONE;
    s_runtime.queue_count--;
    return (int)DM2_TIMELINE_RESULT_OK;
}

/* ── State ──────────────────────────────────────────────────────── */
int dm2_v1_timeline_get_fire_count(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return -1;
    return s_runtime.states[slot].fired_count;
}

int dm2_v1_timeline_is_active(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return 0;
    return s_runtime.states[slot].active;
}

int dm2_v1_timeline_get_event_fire_at(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return -1;
    return s_runtime.queue[slot].fire_at_ms;
}

const DM2_V1_TimelineEventState *dm2_v1_timeline_get_state(int event_id) {
    ensure_init();
    int slot = find_slot_for(event_id);
    if (slot < 0) return NULL;
    return &s_runtime.states[slot];
}

int dm2_v1_timeline_queue_size(void) {
    ensure_init();
    return s_runtime.queue_count;
}

/* ── Observability ──────────────────────────────────────────────── */
int dm2_v1_timeline_total_fires(void) {
    ensure_init();
    return s_runtime.total_fires;
}

int dm2_v1_timeline_total_ticks(void) {
    ensure_init();
    return s_runtime.total_ticks;
}

const char *dm2_v1_timeline_source_evidence(void) {
    return
        "DM2 V1 Timeline Wiring parity - Phase 4 source-lock\n"
        "Source: skproject/SKULLWIN/c_timeline.cpp        (timeline core)\n"
        "Source: skproject/SKWIN/DME.h:1780-1850          (timeline_event_t)\n"
        "Source: skproject/SKWIN/SkGlobal.cpp:1280-1350   (dTimelineTable)\n"
        "Source: ReDMCSB TIMELINE.C:43-220                (F0256/F0261 timeline events)\n"
        "Source: ReDMCSB GAMELOOP.C:69                    (F0261_TIMELINE_Process_CPSEF)\n"
        "Source: ReDMCSB TIMELINE.C:882-908               (door destruction event C02)\n"
        "Event kinds: NPC_MOVE / CREATURE_SPAWN / DOOR_LOCK / DOOR_UNLOCK / MESSAGE_DISPLAY\n"
        "V1 invariant: timeline events mutate world state, NEVER party state.\n";
}
