
#ifndef FIRESTAFF_DM2_V1_TIMELINE_H
#define FIRESTAFF_DM2_V1_TIMELINE_H

/*
 * dm2_v1_timeline.h - DM2 V1 Timeline Wiring Parity
 *
 * DM2 Phase 4 mechanics parity: timeline event scheduling.
 *
 * The DM2 timeline is a time-ordered event queue.  Events are scheduled
 * at absolute or relative times, and the tick walker fires events whose
 * fire_at_ms <= now_ms.  Unlike triggers (which are signal-driven),
 * timeline events are pure time-driven.
 *
 * Source-lock anchors (DM2 decompilation via skproject + ReDMCSB parity):
 *   skproject/SKULLWIN/c_timeline.cpp        - timeline core
 *   skproject/SKWIN/DME.h:1780-1850          - timeline_event_t
 *   skproject/SKWIN/SkGlobal.cpp:1280-1350   - dTimelineTable
 *   ReDMCSB TIMELINE.C:43-220 (DM1 parity)   - F0256/F0261 timeline events
 *   ReDMCSB GAMELOOP.C:69                    - F0261_TIMELINE_Process_CPSEF
 *   ReDMCSB MOVESENS.C:1000-1100             - F0268_SENSOR_AddEvent
 *   ReDMCSB TIMELINE.C:882-908               - door destruction event (C02)
 *   ReDMCSB TIMELINE.C:1459-1482             - teleporter event (C08)
 *
 * Event kinds:
 *   NPC_MOVE         - move NPC to a position
 *   CREATURE_SPAWN   - spawn creature pool entry
 *   DOOR_LOCK        - lock door (state=DESTROYED prevented)
 *   DOOR_UNLOCK      - unlock door
 *   MESSAGE_DISPLAY  - show a string
 *
 * V1 invariant: timeline events mutate world state (doors, creatures,
 * messages) but NEVER party state directly.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ───────────────────────────────────────────────────── */
#define DM2_TIMELINE_MAX_EVENTS        32
#define DM2_TIMELINE_NUM_BUILTIN        6
#define DM2_TIMELINE_NONE               -1  /* "no event" sentinel */

/* Event kinds */
typedef enum {
    DM2_TIMELINE_EVENT_NPC_MOVE       = 1,
    DM2_TIMELINE_EVENT_CREATURE_SPAWN = 2,
    DM2_TIMELINE_EVENT_DOOR_LOCK      = 3,
    DM2_TIMELINE_EVENT_DOOR_UNLOCK    = 4,
    DM2_TIMELINE_EVENT_MESSAGE_DISPLAY = 5,
} DM2_TimelineEventKind;

/* Result codes */
typedef enum {
    DM2_TIMELINE_RESULT_OK = 0,
    DM2_TIMELINE_RESULT_NOT_FOUND,
    DM2_TIMELINE_RESULT_QUEUE_FULL,
    DM2_TIMELINE_RESULT_BAD_KIND,
    DM2_TIMELINE_RESULT_INVALID_TIME,
} DM2_TimelineResult;

/* Event descriptor */
typedef struct {
    int  event_id;
    int  kind;            /* DM2_TIMELINE_EVENT_* */
    int  fire_at_ms;      /* absolute time */
    int  arg_x, arg_y, arg_level;
    int  arg_creature_id;
    int  arg_door_id;
    const char *message;
} DM2_V1_TimelineEvent;

/* Per-event runtime state */
typedef struct {
    int  event_id;
    int  active;
    int  fired_count;
    int  fire_at_ms;
} DM2_V1_TimelineEventState;

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_timeline_reset_state(void);
void dm2_v1_timeline_set_now_ms(int now_ms);
int  dm2_v1_timeline_get_now_ms(void);

/* ── Built-in catalog ───────────────────────────────────────────── */
int  dm2_v1_timeline_get_builtin_count(void);
const DM2_V1_TimelineEvent *dm2_v1_timeline_get_builtin(int event_id);
int  dm2_v1_timeline_lookup_index(int event_id);

/* ── Schedule + tick ────────────────────────────────────────────── */
int  dm2_v1_timeline_init(void);   /* loads built-in events into queue */
int  dm2_v1_timeline_schedule(int event_id, int fire_at_ms);
int  dm2_v1_timeline_tick(int now_ms);   /* fires due events, returns count */
int  dm2_v1_timeline_fire(int event_id);  /* explicit fire */
int  dm2_v1_timeline_cancel(int event_id);

/* ── State ──────────────────────────────────────────────────────── */
int  dm2_v1_timeline_get_fire_count(int event_id);
int  dm2_v1_timeline_is_active(int event_id);
int  dm2_v1_timeline_get_event_fire_at(int event_id);
const DM2_V1_TimelineEventState *dm2_v1_timeline_get_state(int event_id);
int  dm2_v1_timeline_queue_size(void);

/* ── Observability ──────────────────────────────────────────────── */
int  dm2_v1_timeline_total_fires(void);
int  dm2_v1_timeline_total_ticks(void);

const char *dm2_v1_timeline_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIMELINE_H */
