
#ifndef FIRESTAFF_DM2_V1_TRIGGER_H
#define FIRESTAFF_DM2_V1_TRIGGER_H

/*
 * dm2_v1_trigger.h - DM2 V1 Trigger System Parity
 *
 * DM2 Phase 4 mechanics parity: triggers (timeline-fired events).
 *
 * Triggers are scheduled events that fire under specific conditions:
 *   - SQUARE_ENTERED  - party enters a specific square
 *   - ITEM_USED       - champion uses a specific item
 *   - TIME_ELAPSED    - N ms after a reference time
 *   - COMBAT_ENDED    - combat just resolved (party victory/defeat)
 *
 * Source-lock anchors (DM2 decompilation via skproject):
 *   skproject/SKULLWIN/c_trigger.cpp        - trigger system core
 *   skproject/SKWIN/DME.h:1700-1780          - trigger_descriptor_t
 *   skproject/SKWIN/SkGlobal.cpp:1212-1280   - dTriggersTable
 *   ReDMCSB TIMELINE.C:43-220 (DM1 parity)   - F0256/F0261 timeline events
 *   ReDMCSB GAMELOOP.C:69                    - F0261_TIMELINE_Process_CPSEF
 *   ReDMCSB MOVESENS.C:1000-1100             - F0268_SENSOR_AddEvent
 *
 * Each trigger has:
 *   - A kind (when does it fire?)
 *   - A target (what does it do when fired?)
 *   - A fire-once vs repeating flag
 *   - An optional argument (item_id, square coords, time_ms, etc.)
 *
 * Targets:
 *   - DOOR_TOGGLE   - mirror pressure plate
 *   - SPAWN_CREATURE- spawn a creature pool
 *   - DISPLAY_MSG   - show a string
 *   - TELEPORT_PARTY- move party to coords
 *
 * V1 invariant: trigger fires NEVER mutate party state outside the
 * door state machine and the world-state creature pool.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ───────────────────────────────────────────────────── */
#define DM2_TRIGGER_MAX_TRIGGERS     16
#define DM2_TRIGGER_NUM_BUILTIN       8

/* Trigger kinds */
typedef enum {
    DM2_TRIGGER_KIND_SQUARE_ENTERED  = 1,
    DM2_TRIGGER_KIND_ITEM_USED       = 2,
    DM2_TRIGGER_KIND_TIME_ELAPSED    = 3,
    DM2_TRIGGER_KIND_COMBAT_ENDED    = 4,
} DM2_TriggerKind;

/* Trigger targets */
typedef enum {
    DM2_TRIGGER_TARGET_DOOR_TOGGLE   = 1,
    DM2_TRIGGER_TARGET_DOOR_OPEN     = 2,
    DM2_TRIGGER_TARGET_DOOR_CLOSE    = 3,
    DM2_TRIGGER_TARGET_SPAWN_CREATURE = 4,
    DM2_TRIGGER_TARGET_DISPLAY_MSG   = 5,
    DM2_TRIGGER_TARGET_TELEPORT_PARTY = 6,
} DM2_TriggerTarget;

/* Result codes */
typedef enum {
    DM2_TRIGGER_RESULT_OK = 0,
    DM2_TRIGGER_RESULT_NOT_FOUND,
    DM2_TRIGGER_RESULT_DISABLED,
    DM2_TRIGGER_RESULT_ALREADY_FIRED,
    DM2_TRIGGER_RESULT_GUARDED,        /* recursive guard tripped */
    DM2_TRIGGER_RESULT_BAD_KIND,
} DM2_TriggerResult;

/* Trigger descriptor */
typedef struct {
    int  trigger_id;
    int  kind;             /* DM2_TRIGGER_KIND_* */
    int  target;           /* DM2_TRIGGER_TARGET_* */
    int  fire_once;        /* 1 = single shot */
    int  enabled;          /* 1 = active */
    /* Kind-specific arguments */
    int  arg_map_x;
    int  arg_map_y;
    int  arg_map_level;
    int  arg_item_id;
    int  arg_time_ms;
    /* Target arguments */
    int  target_x;
    int  target_y;
    int  target_level;
    int  arg_creature_id;
    const char *message;   /* for DISPLAY_MSG */
} DM2_V1_Trigger;

/* Per-trigger runtime state */
typedef struct {
    int  trigger_id;
    int  active;
    int  fired_count;
    int  firing_now;       /* recursive guard */
    int  last_fire_ms;
} DM2_V1_TriggerState;

/* Last fired target receipt.  This is the startup/runtime handoff shape:
 * callers can observe which target family fired without reaching into the
 * builtin catalog or inferring behavior from counters alone. */
typedef struct {
    int  valid;
    int  trigger_id;
    int  kind;
    int  target;
    int  target_x;
    int  target_y;
    int  target_level;
    int  arg_creature_id;
    int  now_ms;
    int  fire_count;
    const char *message;
} DM2_V1_TriggerEvent;

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_trigger_reset_state(void);
void dm2_v1_trigger_set_now_ms(int now_ms);
int  dm2_v1_trigger_get_now_ms(void);

/* ── Catalog ────────────────────────────────────────────────────── */
int  dm2_v1_trigger_get_builtin_count(void);
const DM2_V1_Trigger *dm2_v1_trigger_get_builtin(int trigger_id);
int  dm2_v1_trigger_lookup_index(int trigger_id);

/* ── Tick + fire API ────────────────────────────────────────────── */
int  dm2_v1_trigger_tick(int now_ms);     /* walks all triggers, fires those due */
int  dm2_v1_trigger_fire(int trigger_id); /* explicit fire */
int  dm2_v1_trigger_signal_square_entered(int x, int y, int level);
int  dm2_v1_trigger_signal_item_used(int item_id);
int  dm2_v1_trigger_signal_combat_ended(int victory);

/* ── State ──────────────────────────────────────────────────────── */
int  dm2_v1_trigger_get_fire_count(int trigger_id);
int  dm2_v1_trigger_is_active(int trigger_id);
const DM2_V1_TriggerState *dm2_v1_trigger_get_state(int trigger_id);
const DM2_V1_TriggerEvent *dm2_v1_trigger_last_event(void);
int  dm2_v1_trigger_copy_last_event(DM2_V1_TriggerEvent *out);

/* ── Observability ──────────────────────────────────────────────── */
int  dm2_v1_trigger_total_fires(void);
int  dm2_v1_trigger_total_signals(void);

const char *dm2_v1_trigger_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TRIGGER_H */
