#ifndef FIRESTAFF_DM2_V1_TIMELINE_H
#define FIRESTAFF_DM2_V1_TIMELINE_H

/*
 * Source-owned DM2 timer queue.
 *
 * SKULLWIN/c_timer.h defines c_tim as a 12-byte save/runtime record.  The
 * queue ordering is taken from c_timer.cpp::DM2_cmp_timers and processing
 * follows c_tim_proc.cpp::DM2_PROCEED_TIMERS.  Timer payload grammar and
 * type-specific effects remain the responsibility of a verified caller; no
 * default NPC, door, creature, or message events are manufactured here.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_SOURCE_TIMER_SIZE 12U
#define DM2_V1_SOURCE_TIMER_MAX 32U
#define DM2_V1_SOURCE_TIMER_TICK_MASK UINT32_C(0x00ffffff)

typedef struct {
    uint32_t ticks_and_map;
    uint8_t type;
    uint8_t actor;
    int16_t value_a;
    int16_t value_b;
    int16_t reserved;
} DM2_V1_SourceTimer;

typedef struct {
    DM2_V1_SourceTimer timers[DM2_V1_SOURCE_TIMER_MAX];
    uint16_t source_indices[DM2_V1_SOURCE_TIMER_MAX];
    size_t count;
} DM2_V1_SourceTimerQueue;

typedef enum {
    DM2_V1_SOURCE_TIMER_OK = 0,
    DM2_V1_SOURCE_TIMER_FULL,
    DM2_V1_SOURCE_TIMER_EMPTY,
    DM2_V1_SOURCE_TIMER_NOT_DUE,
    DM2_V1_SOURCE_TIMER_DISPATCH_UNAVAILABLE,
    DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED
} DM2_V1_SourceTimerResult;

typedef bool (*DM2_V1_SourceTimerDispatch)(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index);

void dm2_v1_source_timer_queue_init(DM2_V1_SourceTimerQueue *queue);
int dm2_v1_source_timer_compare(const DM2_V1_SourceTimer *left,
                                uint16_t left_source_index,
                                const DM2_V1_SourceTimer *right,
                                uint16_t right_source_index);
DM2_V1_SourceTimerResult dm2_v1_source_timer_enqueue(
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index);
bool dm2_v1_source_timer_is_due(const DM2_V1_SourceTimerQueue *queue,
                                uint32_t game_tick);
DM2_V1_SourceTimerResult dm2_v1_source_timer_pop_due(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    DM2_V1_SourceTimer *out_timer,
    uint16_t *out_source_index);
DM2_V1_SourceTimerResult dm2_v1_source_timer_dispatch_due(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    DM2_V1_SourceTimerDispatch dispatch,
    void *context,
    size_t *out_dispatched_count);

const char *dm2_v1_source_timer_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIMELINE_H */
