#ifndef FIRESTAFF_DM2_V1_PROCEED_TIMERS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PROCEED_TIMERS_PC34_COMPAT_H

/*
 * dm2_v1_proceed_timers_pc34_compat.h — DM2 V1 source-ordered timer
 * dispatcher (DM2-003).
 *
 * Every DM2 timer routes through this DM2-owned dispatcher, which mirrors
 * skproject/SKULLWIN/c_tim_proc.cpp DM2_PROCEED_TIMERS (lines 3980-4230):
 * pop the due heap head in DM2_cmp_timers order (c_timer.cpp:31-47),
 * DM2_CHANGE_CURRENT_MAP_TO the timer's map, then dispatch on the timer
 * type matrix.  Host-side behavioural substitution is removed: a known
 * type without a bound DM2-owned handler is acknowledged in source order
 * and counted fail-closed, never simulated.  Unknown types are skipped
 * exactly like the source's `continue` on unmatched RG4UW.
 *
 * Timer layout and queue ordering come from dm2_v1_timeline.h
 * (c_timer.h c_tim 12-byte record; tick ascending, type descending,
 * actor descending, then source index).
 */

#include <stdint.h>
#include "dm2_v1_timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Timer type matrix, c_tim_proc.cpp DM2_PROCEED_TIMERS (3980-4230). */
enum {
    DM2_V1_TIMER_STEP_DOOR = 0x01,             /* DM2_STEP_DOOR */
    DM2_V1_TIMER_DESTROY_DOOR = 0x02,          /* DM2_PROCESS_TIMER_DESTROY_DOOR */
    DM2_V1_TIMER_ACTUATE_TILE = 0x04,          /* tile-class actuator dispatch */
    DM2_V1_TIMER_PROCESS_0C = 0x0c,            /* DM2_PROCESS_TIMER_0C */
    DM2_V1_TIMER_RESURRECTION = 0x0d,          /* DM2_PROCESS_TIMER_RESURRECTION */
    DM2_V1_TIMER_PROCESS_0E = 0x0e,            /* DM2_PROCESS_TIMER_0E */
    DM2_V1_TIMER_PROCESS_SOUND = 0x15,         /* DM2_PROCESS_SOUND */
    DM2_V1_TIMER_PROCESS_CLOUD = 0x19,         /* DM2_PROCESS_CLOUD */
    DM2_V1_TIMER_STEP_MISSILE_ALT = 0x1d,      /* DM2_STEP_MISSILE alias */
    DM2_V1_TIMER_STEP_MISSILE = 0x1e,          /* DM2_STEP_MISSILE */
    DM2_V1_TIMER_THINK_CREATURE_A = 0x21,      /* DM2_THINK_CREATURE */
    DM2_V1_TIMER_THINK_CREATURE_B = 0x22,      /* DM2_THINK_CREATURE */
    DM2_V1_TIMER_PROCESS_3C = 0x3c,            /* DM2_PROCESS_TIMER_3D */
    DM2_V1_TIMER_PROCESS_3D = 0x3d,            /* DM2_PROCESS_TIMER_3D */
    DM2_V1_TIMER_LIGHT = 0x46,                 /* DM2_PROCESS_TIMER_LIGHT */
    DM2_V1_TIMER_HERO_ENCH_FLAG = 0x47,        /* heroflag 0x4000 decrement */
    DM2_V1_TIMER_ENCH_POWER = 0x48,            /* ench_power decrement */
    DM2_V1_TIMER_POISON = 0x4b,                /* DM2_PROCESS_POISON */
    DM2_V1_TIMER_UPDATE_WEATHER = 0x54,        /* DM2_UPDATE_WEATHER(1) */
    DM2_V1_TIMER_ORNATE_ANIMATOR = 0x55,       /* DM2_CONTINUE_ORNATE_ANIMATOR */
    DM2_V1_TIMER_TICK_GENERATOR = 0x56,        /* DM2_CONTINUE_TICK_GENERATOR */
    DM2_V1_TIMER_RELEASE_DOOR_BUTTON = 0x58,   /* DM2_PROCESS_TIMER_RELEASE_DOOR_BUTTON */
    DM2_V1_TIMER_PROCESS_59 = 0x59,            /* DM2_PROCESS_TIMER_59 */
    DM2_V1_TIMER_ORNATE_NOISE = 0x5a,          /* DM2_CONTINUE_ORNATE_NOISE */
    DM2_V1_TIMER_5B_RECORD_CLEAR = 0x5b,       /* record byte@4 &= ~1 */
    DM2_V1_TIMER_5C_RECORD_SET = 0x5c,         /* record byte@2 |= 1 */
    DM2_V1_TIMER_MOVE_RECORD_ROTATE = 0x5d,    /* MOVE_RECORD_TO + party rotate */
    DM2_V1_TIMER_ALLOC_NEW_CREATURE = 0x5e     /* DM2_ALLOC_NEW_CREATURE */
};

#define DM2_V1_TIMER_TYPE_MATRIX_SIZE 0x60
#define DM2_V1_TIMER_ACTUATOR_TILE_CLASSES 7

typedef struct DM2_V1_ProceedTimersReceipt DM2_V1_ProceedTimersReceipt;

/* Return 1 when the handler consumed the timer, 0 to reject (counted,
 * dispatch continues — the source loop never aborts on a timer). */
typedef int (*DM2_V1_TimerTypeHandler)(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt);

typedef struct {
    void *context;
    /* Sparse by timer type (0x00..0x5f).  NULL = no DM2-owned handler:
     * acknowledged in source order and counted fail-closed. */
    DM2_V1_TimerTypeHandler handlers[DM2_V1_TIMER_TYPE_MATRIX_SIZE];
    /* 0x04 subdispatch by source square class 0..6 (c_tim_proc.cpp:4214-4230:
     * 0 wall mecha, 1 floor mecha, 2 pitfall, 3 fall-through no-op,
     * 4 door, 5 teleporter, 6 trickwall). */
    DM2_V1_TimerTypeHandler actuator_tile[DM2_V1_TIMER_ACTUATOR_TILE_CLASSES];
    /* Square-class lookup for the 0x04 boundary: the source reads
     * mapdat.map[x][y] >> 5 directly.  Return the class 0..6, or -1 when
     * the map state is unavailable (fail-closed). */
    int (*tile_class_at)(void *context, int map, int x, int y);
} DM2_V1_TimerDispatcher;

struct DM2_V1_ProceedTimersReceipt {
    int valid;
    uint32_t game_tick;
    int due_count;
    int dispatched_count;         /* handler consumed */
    int skipped_unknown_type;     /* source `continue` on unmatched type */
    int fail_closed_count;        /* known type without a bound handler */
    int handler_rejected_count;   /* handler returned 0 */
    int map_switches;             /* DM2_CHANGE_CURRENT_MAP_TO per pop */
    int last_map;
    int type_tally[DM2_V1_TIMER_TYPE_MATRIX_SIZE];
    int actuator_tile_tally[DM2_V1_TIMER_ACTUATOR_TILE_CLASSES];
};

/* DM2_PROCEED_TIMERS boundary: pop every due timer from `queue` in
 * DM2_cmp_timers order and route it through `dispatcher`.  Returns 1 when
 * the loop ran (even with zero due timers), 0 only for NULL arguments. */
int dm2_v1_proceed_timers(DM2_V1_SourceTimerQueue *queue,
                          uint32_t game_tick,
                          const DM2_V1_TimerDispatcher *dispatcher,
                          DM2_V1_ProceedTimersReceipt *out_receipt);

/* Exact membership test for the source timer-type matrix. */
int dm2_v1_timer_type_is_known(uint8_t type);

const char *dm2_v1_proceed_timers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PROCEED_TIMERS_PC34_COMPAT_H */
