#include "dm2_v1_timeline.h"

#include <assert.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int call_count;
    uint8_t types[DM2_V1_SOURCE_TIMER_MAX];
    uint16_t source_indices[DM2_V1_SOURCE_TIMER_MAX];
    int reject_type;
} dispatch_capture;

static DM2_V1_SourceTimer make_timer(uint32_t tick, uint8_t type,
                                     uint8_t actor)
{
    DM2_V1_SourceTimer timer = { 0U, type, actor, 0, 0, 0 };

    timer.ticks_and_map = tick & DM2_V1_SOURCE_TIMER_TICK_MASK;
    return timer;
}

static __attribute__((unused)) DM2_V1_SourceTimerResult enqueue_timer(DM2_V1_SourceTimerQueue *queue,
                                               uint32_t tick, uint8_t type,
                                               uint8_t actor,
                                               uint16_t source_index)
{
    DM2_V1_SourceTimer timer = make_timer(tick, type, actor);

    return dm2_v1_source_timer_enqueue(queue, &timer, source_index);
}

static __attribute__((unused)) bool capture_dispatch(void *context, const DM2_V1_SourceTimer *timer,
                             uint16_t source_index)
{
    dispatch_capture *capture = context;

    capture->types[capture->call_count] = timer->type;
    capture->source_indices[capture->call_count] = source_index;
    ++capture->call_count;
    return capture->reject_type != (int)timer->type;
}

int main(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer timer;
    (void)timer;
    uint16_t source_index = 0U;
    (void)source_index;
    dispatch_capture capture = { 0U, { 0U }, { 0U }, -1 };
    size_t dispatched = 0U;
    (void)dispatched;

    assert(sizeof(DM2_V1_SourceTimer) == DM2_V1_SOURCE_TIMER_SIZE);
    dm2_v1_source_timer_queue_init(&queue);
    assert(enqueue_timer(&queue, 4U, 1U, 1U, 9U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(enqueue_timer(&queue, 3U, 1U, 1U, 9U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(enqueue_timer(&queue, 3U, 4U, 1U, 7U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(enqueue_timer(&queue, 3U, 4U, 7U, 6U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(enqueue_timer(&queue, 3U, 4U, 7U, 2U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(!dm2_v1_source_timer_is_due(&queue, 2U));
    assert(dm2_v1_source_timer_pop_due(&queue, 2U, NULL, NULL) ==
           DM2_V1_SOURCE_TIMER_NOT_DUE);

    assert(dm2_v1_source_timer_pop_due(&queue, 3U, &timer, &source_index) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(timer.type == 4U && timer.actor == 7U && source_index == 2U);
    assert(dm2_v1_source_timer_pop_due(&queue, 3U, &timer, &source_index) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(timer.type == 4U && timer.actor == 7U && source_index == 6U);
    assert(dm2_v1_source_timer_pop_due(&queue, 3U, &timer, &source_index) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(timer.type == 4U && timer.actor == 1U && source_index == 7U);
    assert(dm2_v1_source_timer_pop_due(&queue, 3U, &timer, &source_index) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(timer.type == 1U && timer.actor == 1U && source_index == 9U);
    assert(dm2_v1_source_timer_pop_due(&queue, 3U, NULL, NULL) ==
           DM2_V1_SOURCE_TIMER_NOT_DUE);

    assert(dm2_v1_source_timer_dispatch_due(&queue, 4U, NULL, &capture,
                                             &dispatched) ==
           DM2_V1_SOURCE_TIMER_DISPATCH_UNAVAILABLE);
    assert(queue.count == 1U && dispatched == 0U);
    assert(dm2_v1_source_timer_dispatch_due(&queue, 4U, capture_dispatch,
                                             &capture, &dispatched) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(dispatched == 1U && capture.call_count == 1U &&
           capture.types[0] == 1U && capture.source_indices[0] == 9U);

    dm2_v1_source_timer_queue_init(&queue);
    capture.reject_type = 2;
    assert(enqueue_timer(&queue, 5U, 2U, 0U, 1U) ==
           DM2_V1_SOURCE_TIMER_OK);
    assert(dm2_v1_source_timer_dispatch_due(&queue, 5U, capture_dispatch,
                                             &capture, &dispatched) ==
           DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED);
    assert(dispatched == 0U && queue.count == 0U);
    assert(enqueue_timer(&queue, 1U, 0U, 0U, 0U) ==
           DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED);
    assert(strstr(dm2_v1_source_timer_source_evidence(),
                  "DM2_cmp_timers") != NULL);
    return 0;
}
