#ifndef FIRESTAFF_DM2_V1_TIMER_QUEUE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TIMER_QUEUE_PC34_COMPAT_H

/*
 * dm2_v1_timer_queue_pc34_compat.h — DM2 timer priority queue (min-heap).
 *
 * Ports the timer queue data structure from skproject sktimer.cpp.
 * This is a binary min-heap keyed on (tick, type, actor) that manages
 * all timed game events: creature AI, spells, light decay, doors, etc.
 *
 * The queue uses an indirection array (indices) so timer entries stay
 * at fixed slots (important for save/load), while heap ordering is
 * maintained via the indices array.
 *
 * Source: skproject/SKWINSPX/src/v5/sktimer.{h,cpp}
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Timer entry — 12 bytes, matching c_tim layout.
 * l_00 packs map in bits 31:24 and tick count in bits 23:0. */
typedef struct {
    int32_t l_00;
    uint8_t ttype;
    uint8_t actor;
    int8_t xA;
    int8_t yA;
    int16_t wvalueB;
    int16_t dummya;
} DM2_V1_TimerEntry;

/* Timer priority queue — binary min-heap with indirection. */
typedef struct {
    DM2_V1_TimerEntry *entries;
    int16_t *indices;
    int16_t num_indices;
    int16_t num_timers;
    int16_t available_idx;
    int16_t max_timers;
    int16_t deferred_sift;
    int32_t gametick;
} DM2_V1_TimerQueue;

/* Accessors for the packed l_00 field. */
static inline int32_t dm2_v1_timer_get_ticks(const DM2_V1_TimerEntry *t) {
    return t->l_00 & 0xffffff;
}
static inline int16_t dm2_v1_timer_get_map(const DM2_V1_TimerEntry *t) {
    return (int16_t)(uint8_t)((t->l_00 >> 24) & 0xff);
}
static inline void dm2_v1_timer_set_mticks(DM2_V1_TimerEntry *t, int16_t map, int32_t tick) {
    t->l_00 = ((int32_t)map << 24) | (tick & 0xffffff);
}
static inline void dm2_v1_timer_inc_data(DM2_V1_TimerEntry *t) {
    t->l_00++;
}

/* Initialize a timer queue. Caller owns entries[] and indices[] arrays. */
void dm2_v1_timer_queue_init(DM2_V1_TimerQueue *q,
                             DM2_V1_TimerEntry *entries,
                             int16_t *indices,
                             int16_t max_timers);

/* Initialize a timer entry to zero/unused. */
void dm2_v1_timer_entry_init(DM2_V1_TimerEntry *t);

/* Sort all timers into heap order and rebuild the free list.
 * Source: sktimer.cpp DM2_SORT_TIMERS + DM2_REARRANGE_TIMERLIST */
void dm2_v1_timer_sort(DM2_V1_TimerQueue *q);

/* Insert a timer into the queue. Returns the slot index, or -1 if
 * the timer has no type (ttype == 0).
 * Source: sktimer.cpp DM2_QUEUE_TIMER */
int16_t dm2_v1_timer_queue(DM2_V1_TimerQueue *q, const DM2_V1_TimerEntry *timer);

/* Remove a timer by slot index.
 * Source: sktimer.cpp DM2_DELETE_TIMER */
void dm2_v1_timer_delete(DM2_V1_TimerQueue *q, int16_t slot);

/* Pop the next (earliest) timer into *out and remove it from the queue.
 * Source: sktimer.cpp DM2_GET_AND_DELETE_NEXT_TIMER */
void dm2_v1_timer_get_and_delete_next(DM2_V1_TimerQueue *q, DM2_V1_TimerEntry *out);

/* Check whether the next timer is due (tick <= gametick).
 * Source: sktimer.cpp DM2_IS_TIMER_TO_PROCEED */
int dm2_v1_timer_is_due(DM2_V1_TimerQueue *q);

/* Re-heapify a timer after its data changed (by slot index).
 * Source: sktimer.cpp DM2_timer_3a15_05f7 */
void dm2_v1_timer_reheapify(DM2_V1_TimerQueue *q, int16_t slot);

/* Find the heap position of a timer by slot index.
 * Returns -1 if not found (caller should treat as error).
 * Source: sktimer.cpp DM2_GET_TIMER_NEW_INDEX */
int16_t dm2_v1_timer_get_heap_index(const DM2_V1_TimerQueue *q, int16_t slot);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIMER_QUEUE_PC34_COMPAT_H */
