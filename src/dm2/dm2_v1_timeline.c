#include "dm2_v1_timeline.h"

#include <string.h>

static uint32_t timer_ticks(const DM2_V1_SourceTimer *timer)
{
    return timer->ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK;
}

void dm2_v1_source_timer_queue_init(DM2_V1_SourceTimerQueue *queue)
{
    if (queue != NULL) {
        memset(queue, 0, sizeof(*queue));
    }
}

int dm2_v1_source_timer_compare(const DM2_V1_SourceTimer *left,
                                uint16_t left_source_index,
                                const DM2_V1_SourceTimer *right,
                                uint16_t right_source_index)
{
    uint32_t left_ticks;
    uint32_t right_ticks;

    if (left == NULL || right == NULL) {
        return 0;
    }
    left_ticks = timer_ticks(left);
    right_ticks = timer_ticks(right);
    if (left_ticks != right_ticks) {
        return left_ticks < right_ticks ? -1 : 1;
    }
    if (left->type != right->type) {
        return left->type > right->type ? -1 : 1;
    }
    if (left->actor != right->actor) {
        return left->actor > right->actor ? -1 : 1;
    }
    if (left_source_index != right_source_index) {
        return left_source_index < right_source_index ? -1 : 1;
    }
    return 0;
}

DM2_V1_SourceTimerResult dm2_v1_source_timer_enqueue(
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index)
{
    DM2_V1_SourceTimerResult result = DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED;

    (void)dm2_v1_source_timer_enqueue_ticketed(queue, timer, source_index,
                                               &result);
    return result;
}

uint32_t dm2_v1_source_timer_enqueue_ticketed(
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_SourceTimerResult *out_result)
{
    size_t position;
    uint32_t ticket;

    if (out_result != NULL) {
        *out_result = DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED;
    }
    if (queue == NULL || timer == NULL || timer->type == 0U) {
        return 0U;
    }
    if (queue->count == DM2_V1_SOURCE_TIMER_MAX) {
        if (out_result != NULL) {
            *out_result = DM2_V1_SOURCE_TIMER_FULL;
        }
        return 0U;
    }
    ticket = ++queue->next_ticket;
    if (ticket == 0U) {
        ticket = ++queue->next_ticket; /* 0 stays the "no timer" value */
    }
    position = queue->count;
    while (position > 0U &&
           dm2_v1_source_timer_compare(timer, source_index,
                                       &queue->timers[position - 1U],
                                       queue->source_indices[position - 1U]) < 0) {
        queue->timers[position] = queue->timers[position - 1U];
        queue->source_indices[position] = queue->source_indices[position - 1U];
        queue->tickets[position] = queue->tickets[position - 1U];
        --position;
    }
    queue->timers[position] = *timer;
    queue->source_indices[position] = source_index;
    queue->tickets[position] = ticket;
    ++queue->count;
    if (out_result != NULL) {
        *out_result = DM2_V1_SOURCE_TIMER_OK;
    }
    return ticket;
}

int dm2_v1_source_timer_cancel(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t ticket)
{
    size_t i;

    if (queue == NULL || ticket == 0U) {
        return 0;
    }
    for (i = 0U; i < queue->count; i++) {
        if (queue->tickets[i] == ticket) {
            if (i + 1U < queue->count) {
                memmove(&queue->timers[i], &queue->timers[i + 1U],
                        (queue->count - 1U - i) * sizeof(queue->timers[0]));
                memmove(&queue->source_indices[i],
                        &queue->source_indices[i + 1U],
                        (queue->count - 1U - i) *
                            sizeof(queue->source_indices[0]));
                memmove(&queue->tickets[i], &queue->tickets[i + 1U],
                        (queue->count - 1U - i) *
                            sizeof(queue->tickets[0]));
            }
            --queue->count;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_source_timer_peek_ticket(
    const DM2_V1_SourceTimerQueue *queue,
    uint32_t ticket,
    DM2_V1_SourceTimer *out_timer)
{
    size_t i;

    if (queue == NULL || out_timer == NULL || ticket == 0U) {
        return 0;
    }
    for (i = 0U; i < queue->count; i++) {
        if (queue->tickets[i] == ticket) {
            *out_timer = queue->timers[i];
            return 1;
        }
    }
    return 0;
}

int dm2_v1_source_timer_update_payload(DM2_V1_SourceTimerQueue *queue,
                                       uint32_t ticket,
                                       int x, int y, int map)
{
    size_t i;

    if (queue == NULL || ticket == 0U) {
        return 0;
    }
    for (i = 0U; i < queue->count; i++) {
        if (queue->tickets[i] == ticket) {
            /* c_timer.h:82 setxyA + c_timer.h:66 setmticks(map,
             * getticks()): valueA lo/hi bytes = x/y, high byte = map,
             * low 24 tick bits preserved. */
            queue->timers[i].value_a =
                (int16_t)((x & 0xff) | ((y & 0xff) << 8));
            queue->timers[i].ticks_and_map =
                ((uint32_t)(map & 0xff) << 24) |
                (queue->timers[i].ticks_and_map &
                 DM2_V1_SOURCE_TIMER_TICK_MASK);
            return 1;
        }
    }
    return 0;
}

bool dm2_v1_source_timer_is_due(const DM2_V1_SourceTimerQueue *queue,
                                uint32_t game_tick)
{
    return queue != NULL && queue->count != 0U &&
           timer_ticks(&queue->timers[0]) <=
               (game_tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
}

DM2_V1_SourceTimerResult dm2_v1_source_timer_pop_due(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    DM2_V1_SourceTimer *out_timer,
    uint16_t *out_source_index)
{
    if (queue == NULL || queue->count == 0U) {
        return DM2_V1_SOURCE_TIMER_EMPTY;
    }
    if (!dm2_v1_source_timer_is_due(queue, game_tick)) {
        return DM2_V1_SOURCE_TIMER_NOT_DUE;
    }
    if (out_timer != NULL) {
        *out_timer = queue->timers[0];
    }
    if (out_source_index != NULL) {
        *out_source_index = queue->source_indices[0];
    }
    if (queue->count > 1U) {
        memmove(&queue->timers[0], &queue->timers[1],
                (queue->count - 1U) * sizeof(queue->timers[0]));
        memmove(&queue->source_indices[0], &queue->source_indices[1],
                (queue->count - 1U) * sizeof(queue->source_indices[0]));
        memmove(&queue->tickets[0], &queue->tickets[1],
                (queue->count - 1U) * sizeof(queue->tickets[0]));
    }
    --queue->count;
    return DM2_V1_SOURCE_TIMER_OK;
}

DM2_V1_SourceTimerResult dm2_v1_source_timer_dispatch_due(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    DM2_V1_SourceTimerDispatch dispatch,
    void *context,
    size_t *out_dispatched_count)
{
    size_t dispatched = 0U;

    if (out_dispatched_count != NULL) {
        *out_dispatched_count = 0U;
    }
    if (dispatch == NULL) {
        return DM2_V1_SOURCE_TIMER_DISPATCH_UNAVAILABLE;
    }
    while (dm2_v1_source_timer_is_due(queue, game_tick)) {
        DM2_V1_SourceTimer timer;
        uint16_t source_index;

        (void)dm2_v1_source_timer_pop_due(queue, game_tick, &timer,
                                           &source_index);
        if (!dispatch(context, &timer, source_index)) {
            if (out_dispatched_count != NULL) {
                *out_dispatched_count = dispatched;
            }
            return DM2_V1_SOURCE_TIMER_DISPATCH_REJECTED;
        }
        ++dispatched;
    }
    if (out_dispatched_count != NULL) {
        *out_dispatched_count = dispatched;
    }
    return DM2_V1_SOURCE_TIMER_OK;
}

const char *dm2_v1_source_timer_source_evidence(void)
{
    return "skproject/SKULLWIN/c_timer.h defines c_tim as 12 bytes: "
           "low 24 bits tick, high byte map, then type, actor, A, B, and "
           "reserved; c_timer.cpp:31-47 DM2_cmp_timers orders tick ascending, "
           "type descending, actor descending, then timer-array address; "
           "c_timer.cpp:261-278 pops the heap head only when due; "
           "c_tim_proc.cpp:3980-4007 dequeues before changing map and "
           "type-specific dispatch.";
}
