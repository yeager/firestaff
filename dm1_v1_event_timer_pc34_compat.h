/*
 * DM1 V1 Event Timer and Scheduler System
 * =========================================
 *
 * Source-locked implementation of the DM1 V1 event queue, matching the
 * original binary min-heap from ReDMCSB TIMELINE.C / DEFS.H / GAMELOOP.C.
 *
 * ReDMCSB source audit (primary references):
 *
 *   DEFS.H:880-920    — EVENT struct (Map_Time, Type+Priority, Location, Cell+Effect)
 *   DEFS.H:922-979    — Event type constants (C00_EVENT_NONE .. C83_EVENT_MAGIC_MAP)
 *   DEFS.H:4266       — UNUSED_EVENT struct
 *   DEFS.H:886        — Type_Priority union for comparison ordering
 *   TIMELINE.C:1-30   — Global variables: G0370_ps_Events, G0371_pui_Timeline,
 *                        G0372_ui_EventCount, G0373_ui_FirstUnusedEventIndex
 *   TIMELINE.C:F0233  — Initialize: allocate events + timeline arrays, zero all types
 *   TIMELINE.C:F0234  — IsEventABeforeEventB: time first, then type (descending),
 *                        then priority (descending), then address (ascending)
 *   TIMELINE.C:F0236  — FixPlacement: binary heap sift-up then sift-down
 *   TIMELINE.C:F0237  — DeleteEvent: mark NONE, decrements count, fixes heap
 *   TIMELINE.C:F0238  — AddEvent: merge door/corridor duplicates, insert into heap
 *   TIMELINE.C:F0239  — ExtractFirstEvent: copy timeline[0] event, delete it
 *   TIMELINE.C:F0240  — IsFirstEventExpired: TIME(events[timeline[0]]) <= GameTime
 *   TIMELINE.C:F0261  — Process: while expired, extract + dispatch by type
 *   GAMELOOP.C:F0002  — GameLoop: calls F0261 each tick, then GameTime++
 *
 * Architecture:
 *   - Binary min-heap in timeline[] (indices into events[])
 *   - Separate events[] array with free-slot tracking
 *   - Event comparison: time -> type (higher first) -> priority -> index
 *   - Original DM1 V1 event types preserved as constants
 *   - Merge logic for door/corridor/wall events matching original
 */

#ifndef DM1_V1_EVENT_TIMER_PC34_COMPAT_H
#define DM1_V1_EVENT_TIMER_PC34_COMPAT_H

#include <stdint.h>

/* ================================================================
 *  Event type constants (ReDMCSB DEFS.H:932-979)
 * ================================================================ */

#define DM1_EVENT_NONE                              0
#define DM1_EVENT_DOOR_ANIMATION                    1
#define DM1_EVENT_DOOR_DESTRUCTION                  2
#define DM1_EVENT_CORRIDOR                          5
#define DM1_EVENT_WALL                              6
#define DM1_EVENT_FAKEWALL                          7
#define DM1_EVENT_TELEPORTER                        8
#define DM1_EVENT_PIT                               9
#define DM1_EVENT_DOOR                             10
#define DM1_EVENT_ENABLE_CHAMPION_ACTION           11
#define DM1_EVENT_HIDE_DAMAGE_RECEIVED             12
#define DM1_EVENT_VI_ALTAR_REBIRTH                 13
#define DM1_EVENT_PLAY_SOUND                       20
#define DM1_EVENT_CPSE                             22
#define DM1_EVENT_REMOVE_FLUXCAGE                  24
#define DM1_EVENT_EXPLOSION                        25
#define DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE  29
#define DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE 30
#define DM1_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT 31
#define DM1_EVENT_UPDATE_ASPECT_GROUP              32
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_0         33
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_1         34
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_2         35
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_3         36
#define DM1_EVENT_UPDATE_BEHAVIOR_GROUP            37
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0       38
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1       39
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2       40
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3       41
#define DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS   48
#define DM1_EVENT_MOVE_PROJECTILE                  49
#define DM1_EVENT_WATCHDOG                         53
#define DM1_EVENT_MOVE_GROUP_SILENT                60
#define DM1_EVENT_MOVE_GROUP_AUDIBLE               61
#define DM1_EVENT_ENABLE_GROUP_GENERATOR           65
#define DM1_EVENT_LIGHT                            70
#define DM1_EVENT_INVISIBILITY                     71
#define DM1_EVENT_CHAMPION_SHIELD                  72
#define DM1_EVENT_THIEVES_EYE                      73
#define DM1_EVENT_PARTY_SHIELD                     74
#define DM1_EVENT_POISON_CHAMPION                  75
#define DM1_EVENT_SPELLSHIELD                      77
#define DM1_EVENT_FIRESHIELD                       78
#define DM1_EVENT_FOOTPRINTS                       79

/* Sensor/effect constants (DEFS.H:1286-1291) */
#define DM1_EFFECT_SET     0
#define DM1_EFFECT_CLEAR   1
#define DM1_EFFECT_TOGGLE  2
#define DM1_EFFECT_HOLD    3

/* ================================================================
 *  Capacities
 * ================================================================ */

#define DM1_EVENT_MAX_COUNT   256
#define DM1_EVENT_SERIALIZED_SIZE  16

/* ================================================================
 *  DM1_Event_V1 — matches ReDMCSB EVENT struct (DEFS.H:880-920)
 * ================================================================ */

struct DM1_Event_V1 {
    uint32_t map_time;      /* bits 31-24: mapIndex, bits 23-0: time */
    uint8_t  type;          /* event type (0..83) */
    uint8_t  priority;      /* priority within same time+type */
    uint8_t  b_mapX;
    uint8_t  b_mapY;
    uint8_t  c_cell;
    uint8_t  c_effect;
};

/* ================================================================
 *  DM1_EventQueue_V1 — Binary min-heap event queue
 * ================================================================ */

struct DM1_EventQueue_V1 {
    uint32_t gameTick;
    int      eventCount;
    int      firstUnusedIndex;
    int      maxEvents;
    struct DM1_Event_V1  events[DM1_EVENT_MAX_COUNT];
    uint16_t timeline[DM1_EVENT_MAX_COUNT];
};

/* ================================================================
 *  Dispatch result
 * ================================================================ */

#define DM1_DISPATCH_NONE             0
#define DM1_DISPATCH_DOOR_ANIMATION   1
#define DM1_DISPATCH_DOOR_DESTRUCTION 2
#define DM1_DISPATCH_SQUARE_EFFECT    3
#define DM1_DISPATCH_MOVE_GROUP       4
#define DM1_DISPATCH_PROJECTILE       5
#define DM1_DISPATCH_EXPLOSION        6
#define DM1_DISPATCH_CHAMPION_ACTION  7
#define DM1_DISPATCH_PARTY_SPELL      8
#define DM1_DISPATCH_SOUND            9
#define DM1_DISPATCH_CREATURE_AI     10
#define DM1_DISPATCH_GENERATOR       11
#define DM1_DISPATCH_FLUXCAGE        12
#define DM1_DISPATCH_WATCHDOG        13
#define DM1_DISPATCH_VI_ALTAR        14
#define DM1_DISPATCH_UNSUPPORTED     15

#define DM1_DISPATCH_MAX_PER_TICK    64

struct DM1_DispatchRecord_V1 {
    int  dispatchKind;
    int  eventType;
    int  mapIndex;
    int  mapX;
    int  mapY;
    int  cell;
    int  effect;
    int  aux0;
    int  aux1;
};

struct DM1_TickDispatchResult_V1 {
    int count;
    struct DM1_DispatchRecord_V1 records[DM1_DISPATCH_MAX_PER_TICK];
};

/* ================================================================
 *  Map_Time macros (DEFS.H:928-932)
 * ================================================================ */

#define DM1_MAP_TIME_MAP(mt)          ((uint8_t)((mt) >> 24))
#define DM1_MAP_TIME_TIME(mt)         ((mt) & 0x00FFFFFFu)
#define DM1_MAP_TIME_SET_MAP(mt, m)   ((mt) = (((mt) & 0x00FFFFFFu) | ((uint32_t)(m) << 24)))
#define DM1_MAP_TIME_SET_TIME(mt, t)  ((mt) = (((mt) & 0xFF000000u) | ((t) & 0x00FFFFFFu)))
#define DM1_MAP_TIME_MAKE(m, t)       (((uint32_t)(m) << 24) | ((t) & 0x00FFFFFFu))

/* ================================================================
 *  API — Queue management
 * ================================================================ */

int dm1v1_event_queue_init(
    struct DM1_EventQueue_V1* queue,
    uint32_t initialGameTick);

int dm1v1_event_add(
    struct DM1_EventQueue_V1* queue,
    const struct DM1_Event_V1* event);

int dm1v1_event_delete(
    struct DM1_EventQueue_V1* queue,
    int eventIndex);

int dm1v1_event_extract_first(
    struct DM1_EventQueue_V1* queue,
    struct DM1_Event_V1* outEvent);

int dm1v1_event_is_first_expired(
    const struct DM1_EventQueue_V1* queue);

int dm1v1_event_get_timeline_index(
    const struct DM1_EventQueue_V1* queue,
    int eventIndex);

/* ================================================================
 *  API — Timer tick processing
 * ================================================================ */

int dm1v1_event_process_tick(
    struct DM1_EventQueue_V1* queue,
    struct DM1_TickDispatchResult_V1* outResult);

void dm1v1_event_advance_tick(
    struct DM1_EventQueue_V1* queue);

/* ================================================================
 *  API — Serialisation
 * ================================================================ */

#define DM1_EVENT_QUEUE_SERIALIZED_SIZE \
    (12 + DM1_EVENT_MAX_COUNT * DM1_EVENT_SERIALIZED_SIZE + DM1_EVENT_MAX_COUNT * 2)

int dm1v1_event_queue_serialize(
    const struct DM1_EventQueue_V1* queue,
    unsigned char* outBuf,
    int outBufSize);

int dm1v1_event_queue_deserialize(
    struct DM1_EventQueue_V1* queue,
    const unsigned char* buf,
    int bufSize);

#endif /* DM1_V1_EVENT_TIMER_PC34_COMPAT_H */
