/*
 * DM1 V1 Event Timer and Scheduler System — Implementation
 *
 * Source-locked to ReDMCSB TIMELINE.C. Key function mappings:
 *
 *   dm1v1_event_queue_init       ← F0233_TIMELINE_Initialize_CPSE
 *   dm1v1_event_is_before        ← F0234_TIMELINE_IsEventABeforeEventB
 *   dm1v1_event_get_timeline_index ← F0235_TIMELINE_GetIndex
 *   dm1v1_event_fix_placement    ← F0236_TIMELINE_FixPlacement
 *   dm1v1_event_delete           ← F0237_TIMELINE_DeleteEvent
 *   dm1v1_event_add              ← F0238_TIMELINE_AddEvent_GetEventIndex_CPSE
 *   dm1v1_event_extract_first    ← F0239_TIMELINE_ExtractFirstEvent
 *   dm1v1_event_is_first_expired ← F0240_TIMELINE_IsFirstEventExpired_CPSE
 *   dm1v1_event_dispatch_one     ← F0261_TIMELINE_Process_CPSEF (switch body)
 *   dm1v1_event_process_tick     ← F0261_TIMELINE_Process_CPSEF (loop)
 */

#include "dm1_v1_event_timer_pc34_compat.h"
#include <string.h>

/* ================================================================
 *  LE serialisation helpers
 * ================================================================ */

static void w_u32(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}
static uint32_t r_u32(const unsigned char* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}
static void w_u16(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
}
static uint16_t r_u16(const unsigned char* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static void w_i32(unsigned char* p, int32_t v) { w_u32(p, (uint32_t)v); }
static int32_t r_i32(const unsigned char* p) { return (int32_t)r_u32(p); }

/* ================================================================
 *  F0234 — IsEventABeforeEventB
 *
 *  ReDMCSB TIMELINE.C: Compare by time first. If simultaneous,
 *  higher Type_Priority value wins (processed first). If still
 *  tied, lower memory address (= lower index) wins.
 *
 *  Type_Priority is a uint16_t where Type is the high byte and
 *  Priority is the low byte on big-endian (original 68k layout).
 *  For comparison purposes: type > priority ordering means type
 *  is compared first (higher type = earlier), then priority.
 *
 *  Source: TIMELINE.C lines for MEDIA203/204 (C code path):
 *    return (TIME(A) < TIME(B)) ||
 *           (simultaneous && (A->Type_Priority > B->Type_Priority)) ||
 *           (simultaneous && (A->Type_Priority == B->Type_Priority) && (A <= B));
 * ================================================================ */

static int dm1v1_event_is_before(
    const struct DM1_Event_V1* a, int indexA,
    const struct DM1_Event_V1* b, int indexB)
{
    uint32_t timeA = DM1_MAP_TIME_TIME(a->map_time);
    uint32_t timeB = DM1_MAP_TIME_TIME(b->map_time);
    uint16_t tpA, tpB;

    if (timeA < timeB) return 1;
    if (timeA != timeB) return 0;

    /* Simultaneous — compare Type_Priority (type is high byte, priority is low) */
    tpA = (uint16_t)((uint16_t)a->type << 8) | (uint16_t)a->priority;
    tpB = (uint16_t)((uint16_t)b->type << 8) | (uint16_t)b->priority;
    if (tpA > tpB) return 1;
    if (tpA < tpB) return 0;

    /* Last resort: lower index wins (mirrors memory address comparison) */
    return indexA <= indexB;
}

/* ================================================================
 *  F0236 — FixPlacement (binary heap sift)
 *
 *  The timeline is a binary min-heap where parent at index i has
 *  children at 2i+1 and 2i+2. "Min" means the earliest event is
 *  at index 0.
 *
 *  Sift up first (toward root), then sift down if no upward move.
 * ================================================================ */

static void dm1v1_event_fix_placement(
    struct DM1_EventQueue_V1* queue,
    int timelineIndex)
{
    int parentIndex;
    int childIndex;
    int halfIndex;
    int eventIndex;
    int placementFixed = 0;

    if (queue->eventCount <= 1) return;

    eventIndex = queue->timeline[timelineIndex];

    /* Sift up */
    while (timelineIndex > 0) {
        parentIndex = (timelineIndex - 1) >> 1;
        if (dm1v1_event_is_before(
                &queue->events[eventIndex], eventIndex,
                &queue->events[queue->timeline[parentIndex]],
                queue->timeline[parentIndex])) {
            queue->timeline[timelineIndex] = queue->timeline[parentIndex];
            timelineIndex = parentIndex;
            placementFixed = 1;
        } else {
            break;
        }
    }

    if (placementFixed) {
        queue->timeline[timelineIndex] = (uint16_t)eventIndex;
        return;
    }

    /* Sift down */
    halfIndex = ((queue->eventCount - 1) - 1) >> 1;
    while (timelineIndex <= halfIndex) {
        childIndex = (timelineIndex << 1) + 1;  /* Left child */
        /* Pick the earlier child */
        if ((childIndex + 1) < queue->eventCount &&
            dm1v1_event_is_before(
                &queue->events[queue->timeline[childIndex + 1]],
                queue->timeline[childIndex + 1],
                &queue->events[queue->timeline[childIndex]],
                queue->timeline[childIndex])) {
            childIndex++;
        }
        if (dm1v1_event_is_before(
                &queue->events[queue->timeline[childIndex]],
                queue->timeline[childIndex],
                &queue->events[eventIndex], eventIndex)) {
            queue->timeline[timelineIndex] = queue->timeline[childIndex];
            timelineIndex = childIndex;
        } else {
            break;
        }
    }
    queue->timeline[timelineIndex] = (uint16_t)eventIndex;
}

/* ================================================================
 *  F0233 — Initialize
 * ================================================================ */

int dm1v1_event_queue_init(
    struct DM1_EventQueue_V1* queue,
    uint32_t initialGameTick)
{
    int i;
    if (!queue) return 0;
    memset(queue, 0, sizeof(*queue));
    queue->gameTick = initialGameTick;
    queue->maxEvents = DM1_EVENT_MAX_COUNT;
    queue->eventCount = 0;
    queue->firstUnusedIndex = 0;
    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        queue->events[i].type = DM1_EVENT_NONE;
    }
    return 1;
}

/* ================================================================
 *  F0235 — GetIndex (find timeline index for an event index)
 * ================================================================ */

int dm1v1_event_get_timeline_index(
    const struct DM1_EventQueue_V1* queue,
    int eventIndex)
{
    int i;
    if (!queue) return -1;
    for (i = 0; i < queue->eventCount; i++) {
        if (queue->timeline[i] == (uint16_t)eventIndex) {
            return i;
        }
    }
    return -1;  /* Not found — error */
}

/* ================================================================
 *  F0237 — DeleteEvent
 * ================================================================ */

int dm1v1_event_delete(
    struct DM1_EventQueue_V1* queue,
    int eventIndex)
{
    int timelineIndex;
    int lastEventCount;

    if (!queue) return 0;
    if (eventIndex < 0 || eventIndex >= queue->maxEvents) return 0;

    queue->events[eventIndex].type = DM1_EVENT_NONE;

    /* Track first unused: ReDMCSB keeps lowest free index */
    if (eventIndex < queue->firstUnusedIndex) {
        queue->firstUnusedIndex = eventIndex;
    }

    lastEventCount = --queue->eventCount;
    if (lastEventCount == 0) return 1;

    timelineIndex = dm1v1_event_get_timeline_index(queue, eventIndex);
    if (timelineIndex < 0) return 0;

    if (timelineIndex == lastEventCount) {
        /* Deleted event was last in timeline — no fix needed */
        return 1;
    }

    /* Replace deleted entry with last entry, then fix */
    queue->timeline[timelineIndex] = queue->timeline[lastEventCount];
    dm1v1_event_fix_placement(queue, timelineIndex);
    return 1;
}

/* ================================================================
 *  F0238 — AddEvent (with door/corridor event merge)
 *
 *  ReDMCSB TIMELINE.C F0238: Before adding certain event types,
 *  scan existing events for merge opportunities:
 *
 *  1. Events C05..C10 (CORRIDOR..DOOR): if an existing C05..C10
 *     event has the same Map_Time and MapXY (and for WALL events,
 *     same Cell), merge by updating Effect. Also check for
 *     DOOR_ANIMATION conflict (toggle resolution).
 *
 *  2. DOOR_ANIMATION (C01): if existing DOOR event has same
 *     Map_Time+MapXY, resolve toggle; if existing DOOR_ANIMATION,
 *     merge Effect.
 *
 *  3. DOOR_DESTRUCTION (C02): delete conflicting DOOR_ANIMATION
 *     or DOOR events on same square.
 * ================================================================ */

int dm1v1_event_add(
    struct DM1_EventQueue_V1* queue,
    const struct DM1_Event_V1* event)
{
    int newIndex;
    int i;
    struct DM1_Event_V1 ev;

    if (!queue || !event) return -1;
    if (queue->eventCount >= queue->maxEvents) return -1;

    ev = *event;

    /* --- Merge logic for corridor/wall/door square events (C05..C10) --- */
    if (ev.type >= DM1_EVENT_CORRIDOR && ev.type <= DM1_EVENT_DOOR) {
        for (i = 0; i < queue->maxEvents; i++) {
            struct DM1_Event_V1* existing = &queue->events[i];
            if (existing->type >= DM1_EVENT_CORRIDOR && existing->type <= DM1_EVENT_DOOR) {
                if (ev.map_time == existing->map_time &&
                    ev.b_mapX == existing->b_mapX &&
                    ev.b_mapY == existing->b_mapY &&
                    (existing->type != DM1_EVENT_WALL || existing->c_cell == ev.c_cell)) {
                    /* Merge: update effect, return existing index */
                    existing->c_effect = ev.c_effect;
                    return i;
                }
            } else if (existing->type == DM1_EVENT_DOOR_ANIMATION) {
                if (ev.map_time == existing->map_time &&
                    ev.b_mapX == existing->b_mapX &&
                    ev.b_mapY == existing->b_mapY) {
                    /* Toggle resolution */
                    if (ev.c_effect == DM1_EFFECT_TOGGLE) {
                        ev.c_effect = 1 - existing->c_effect;
                    }
                    dm1v1_event_delete(queue, i);
                    break;
                }
            }
        }
    }
    /* --- Merge logic for DOOR_ANIMATION (C01) --- */
    else if (ev.type == DM1_EVENT_DOOR_ANIMATION) {
        for (i = 0; i < queue->maxEvents; i++) {
            struct DM1_Event_V1* existing = &queue->events[i];
            if (ev.map_time == existing->map_time &&
                ev.b_mapX == existing->b_mapX &&
                ev.b_mapY == existing->b_mapY) {
                if (existing->type == DM1_EVENT_DOOR) {
                    if (existing->c_effect == DM1_EFFECT_TOGGLE) {
                        existing->c_effect = 1 - ev.c_effect;
                    }
                    return i;
                }
                if (existing->type == DM1_EVENT_DOOR_ANIMATION) {
                    existing->c_effect = ev.c_effect;
                    return i;
                }
            }
        }
    }
    /* --- Merge logic for DOOR_DESTRUCTION (C02) --- */
    else if (ev.type == DM1_EVENT_DOOR_DESTRUCTION) {
        for (i = 0; i < queue->maxEvents; i++) {
            struct DM1_Event_V1* existing = &queue->events[i];
            if (ev.b_mapX == existing->b_mapX &&
                ev.b_mapY == existing->b_mapY &&
                DM1_MAP_TIME_MAP(ev.map_time) == DM1_MAP_TIME_MAP(existing->map_time)) {
                if (existing->type == DM1_EVENT_DOOR_ANIMATION ||
                    existing->type == DM1_EVENT_DOOR) {
                    dm1v1_event_delete(queue, i);
                }
            }
        }
    }

    /* Find free slot */
    newIndex = queue->firstUnusedIndex;
    if (newIndex >= queue->maxEvents) return -1;

    queue->events[newIndex] = ev;

    /* Advance firstUnusedIndex to next free slot */
    do {
        queue->firstUnusedIndex++;
        if (queue->firstUnusedIndex >= queue->maxEvents) break;
    } while (queue->events[queue->firstUnusedIndex].type != DM1_EVENT_NONE);

    /* Insert into timeline heap (eventCount incremented BEFORE fix_placement,
     * matching ReDMCSB where F0236 reads the global G0372_ui_EventCount
     * which is already incremented by the G0372_ui_EventCount++ expression
     * in F0238) */
    {
        int insertPos = queue->eventCount;
        queue->timeline[insertPos] = (uint16_t)newIndex;
        queue->eventCount++;
        dm1v1_event_fix_placement(queue, insertPos);
    }

    return newIndex;
}

/* ================================================================
 *  F0239 — ExtractFirstEvent
 * ================================================================ */

int dm1v1_event_extract_first(
    struct DM1_EventQueue_V1* queue,
    struct DM1_Event_V1* outEvent)
{
    int eventIndex;
    if (!queue || !outEvent) return 0;
    if (queue->eventCount <= 0) return 0;

    eventIndex = queue->timeline[0];
    *outEvent = queue->events[eventIndex];
    dm1v1_event_delete(queue, eventIndex);
    return 1;
}

/* ================================================================
 *  F0240 — IsFirstEventExpired
 * ================================================================ */

int dm1v1_event_is_first_expired(
    const struct DM1_EventQueue_V1* queue)
{
    uint32_t eventTime;
    if (!queue) return 0;
    if (queue->eventCount <= 0) return 0;
    eventTime = DM1_MAP_TIME_TIME(queue->events[queue->timeline[0]].map_time);
    return eventTime <= queue->gameTick;
}

/* ================================================================
 *  Event type → dispatch kind classification
 *
 *  Based on F0261_TIMELINE_Process_CPSEF switch cases.
 * ================================================================ */

static int dm1v1_classify_event(int eventType)
{
    /* Group reactions and creature AI: C29..C41 */
    if (eventType >= DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE &&
        eventType <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        return DM1_DISPATCH_CREATURE_AI;
    }

    switch (eventType) {
    case DM1_EVENT_DOOR_ANIMATION:
        return DM1_DISPATCH_DOOR_ANIMATION;
    case DM1_EVENT_DOOR_DESTRUCTION:
        return DM1_DISPATCH_DOOR_DESTRUCTION;
    case DM1_EVENT_CORRIDOR:
    case DM1_EVENT_WALL:
    case DM1_EVENT_FAKEWALL:
    case DM1_EVENT_TELEPORTER:
    case DM1_EVENT_PIT:
    case DM1_EVENT_DOOR:
        return DM1_DISPATCH_SQUARE_EFFECT;
    case DM1_EVENT_ENABLE_CHAMPION_ACTION:
        return DM1_DISPATCH_CHAMPION_ACTION;
    case DM1_EVENT_HIDE_DAMAGE_RECEIVED:
        return DM1_DISPATCH_CHAMPION_ACTION;
    case DM1_EVENT_VI_ALTAR_REBIRTH:
        return DM1_DISPATCH_VI_ALTAR;
    case DM1_EVENT_PLAY_SOUND:
        return DM1_DISPATCH_SOUND;
    case DM1_EVENT_REMOVE_FLUXCAGE:
        return DM1_DISPATCH_FLUXCAGE;
    case DM1_EVENT_EXPLOSION:
        return DM1_DISPATCH_EXPLOSION;
    case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
    case DM1_EVENT_MOVE_PROJECTILE:
        return DM1_DISPATCH_PROJECTILE;
    case DM1_EVENT_WATCHDOG:
        return DM1_DISPATCH_WATCHDOG;
    case DM1_EVENT_MOVE_GROUP_SILENT:
    case DM1_EVENT_MOVE_GROUP_AUDIBLE:
        return DM1_DISPATCH_MOVE_GROUP;
    case DM1_EVENT_ENABLE_GROUP_GENERATOR:
        return DM1_DISPATCH_GENERATOR;
    case DM1_EVENT_LIGHT:
    case DM1_EVENT_INVISIBILITY:
    case DM1_EVENT_CHAMPION_SHIELD:
    case DM1_EVENT_THIEVES_EYE:
    case DM1_EVENT_PARTY_SHIELD:
    case DM1_EVENT_POISON_CHAMPION:
    case DM1_EVENT_SPELLSHIELD:
    case DM1_EVENT_FIRESHIELD:
    case DM1_EVENT_FOOTPRINTS:
        return DM1_DISPATCH_PARTY_SPELL;
    default:
        return DM1_DISPATCH_UNSUPPORTED;
    }
}

/* ================================================================
 *  F0261 — Process all expired events (event loop body)
 *
 *  This is the core tick dispatch. For each expired event:
 *  1. Extract it from the queue
 *  2. Classify by type
 *  3. Record in dispatch result for caller to act on
 *
 *  The actual side-effect execution (door state mutation, creature
 *  movement, etc.) is delegated to the caller via the dispatch
 *  records — this layer only handles queue management and
 *  classification, matching the pure-data approach of the
 *  existing Firestaff codebase.
 * ================================================================ */

int dm1v1_event_process_tick(
    struct DM1_EventQueue_V1* queue,
    struct DM1_TickDispatchResult_V1* outResult)
{
    struct DM1_Event_V1 ev;
    struct DM1_DispatchRecord_V1* rec;
    int safetyLimit;

    if (!queue || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    safetyLimit = DM1_EVENT_MAX_COUNT * 2;  /* prevent infinite loops */

    while (dm1v1_event_is_first_expired(queue) && safetyLimit > 0) {
        safetyLimit--;

        if (!dm1v1_event_extract_first(queue, &ev)) break;
        if (outResult->count >= DM1_DISPATCH_MAX_PER_TICK) break;

        rec = &outResult->records[outResult->count];
        memset(rec, 0, sizeof(*rec));
        rec->eventType = ev.type;
        rec->mapIndex  = DM1_MAP_TIME_MAP(ev.map_time);
        rec->mapX      = ev.b_mapX;
        rec->mapY      = ev.b_mapY;
        rec->cell      = ev.c_cell;
        rec->effect    = ev.c_effect;
        rec->aux0      = ev.priority;
        rec->aux1      = 0;
        rec->dispatchKind = dm1v1_classify_event(ev.type);

        outResult->count++;
    }

    return outResult->count;
}

/* ================================================================
 *  Advance game tick (corresponds to G0313_ul_GameTime++ in GAMELOOP.C)
 * ================================================================ */

void dm1v1_event_advance_tick(
    struct DM1_EventQueue_V1* queue)
{
    if (!queue) return;
    queue->gameTick++;
}

/* ================================================================
 *  Serialisation
 * ================================================================ */

int dm1v1_event_queue_serialize(
    const struct DM1_EventQueue_V1* queue,
    unsigned char* outBuf,
    int outBufSize)
{
    int i;
    int off = 0;
    unsigned char* p;

    if (!queue || !outBuf) return 0;
    if (outBufSize < DM1_EVENT_QUEUE_SERIALIZED_SIZE) return 0;

    memset(outBuf, 0, DM1_EVENT_QUEUE_SERIALIZED_SIZE);

    /* Header: gameTick(4), eventCount(4), firstUnusedIndex(4) */
    w_u32(outBuf + 0, queue->gameTick);
    w_i32(outBuf + 4, queue->eventCount);
    w_i32(outBuf + 8, queue->firstUnusedIndex);
    off = 12;

    /* Events array */
    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        p = outBuf + off;
        w_u32(p + 0, queue->events[i].map_time);
        p[4] = queue->events[i].type;
        p[5] = queue->events[i].priority;
        p[6] = queue->events[i].b_mapX;
        p[7] = queue->events[i].b_mapY;
        p[8] = queue->events[i].c_cell;
        p[9] = queue->events[i].c_effect;
        /* 10-15: padding zeros */
        off += DM1_EVENT_SERIALIZED_SIZE;
    }

    /* Timeline array */
    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        w_u16(outBuf + off, queue->timeline[i]);
        off += 2;
    }

    return 1;
}

int dm1v1_event_queue_deserialize(
    struct DM1_EventQueue_V1* queue,
    const unsigned char* buf,
    int bufSize)
{
    int i;
    int off = 0;
    const unsigned char* p;

    if (!queue || !buf) return 0;
    if (bufSize < DM1_EVENT_QUEUE_SERIALIZED_SIZE) return 0;

    memset(queue, 0, sizeof(*queue));
    queue->maxEvents = DM1_EVENT_MAX_COUNT;

    queue->gameTick = r_u32(buf + 0);
    queue->eventCount = r_i32(buf + 4);
    queue->firstUnusedIndex = r_i32(buf + 8);
    off = 12;

    if (queue->eventCount < 0 || queue->eventCount > DM1_EVENT_MAX_COUNT) return 0;
    if (queue->firstUnusedIndex < 0 || queue->firstUnusedIndex > DM1_EVENT_MAX_COUNT) return 0;

    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        p = buf + off;
        queue->events[i].map_time  = r_u32(p + 0);
        queue->events[i].type      = p[4];
        queue->events[i].priority  = p[5];
        queue->events[i].b_mapX    = p[6];
        queue->events[i].b_mapY    = p[7];
        queue->events[i].c_cell    = p[8];
        queue->events[i].c_effect  = p[9];
        off += DM1_EVENT_SERIALIZED_SIZE;
    }

    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        queue->timeline[i] = r_u16(buf + off);
        off += 2;
    }

    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining TIMELINE.C function citations for parity
 *
 *   TIMELINE.C:1921 F0210_CPSE_P
 *   TIMELINE.C:43 F0256_TIMELINE_P
 *   TIMELINE.C:525 F0277_CPSE_I
 * ══════════════════════════════════════════════════════════════════════ */

