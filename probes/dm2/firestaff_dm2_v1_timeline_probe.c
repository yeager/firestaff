/* Raw DM2 c_tim queue receipt. See skproject/SKULLWIN/c_timer.cpp. */
#include "dm2_v1_timeline.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int count;
    uint8_t types[DM2_V1_SOURCE_TIMER_MAX];
} dispatch_receipt;

static DM2_V1_SourceTimer source_timer(uint32_t tick, uint8_t type,
                                       uint8_t actor)
{
    DM2_V1_SourceTimer timer = { tick, type, actor, 0, 0, 0 };

    return timer;
}

static int enqueue(DM2_V1_SourceTimerQueue *queue, uint32_t tick,
                   uint8_t type, uint8_t actor, uint16_t source_index)
{
    DM2_V1_SourceTimer timer = source_timer(tick, type, actor);

    return dm2_v1_source_timer_enqueue(queue, &timer, source_index) ==
           DM2_V1_SOURCE_TIMER_OK;
}

static bool receipt_dispatch(void *context, const DM2_V1_SourceTimer *timer,
                             uint16_t source_index)
{
    dispatch_receipt *receipt = context;

    (void)source_index;
    receipt->types[receipt->count++] = timer->type;
    return true;
}

int main(void)
{
    DM2_V1_SourceTimerQueue queue;
    dispatch_receipt receipt = { 0U, { 0U } };
    size_t dispatched = 0U;

    dm2_v1_source_timer_queue_init(&queue);
    if (sizeof(DM2_V1_SourceTimer) != DM2_V1_SOURCE_TIMER_SIZE ||
        !enqueue(&queue, 8U, 1U, 1U, 7U) ||
        !enqueue(&queue, 5U, 2U, 2U, 4U) ||
        !enqueue(&queue, 5U, 2U, 7U, 3U) ||
        !enqueue(&queue, 5U, 4U, 0U, 9U)) {
        return 1;
    }
    if (dm2_v1_source_timer_dispatch_due(&queue, 4U, receipt_dispatch,
                                         &receipt, &dispatched) !=
            DM2_V1_SOURCE_TIMER_OK ||
        dispatched != 0U || receipt.count != 0U) {
        return 1;
    }
    if (dm2_v1_source_timer_dispatch_due(&queue, 5U, receipt_dispatch,
                                         &receipt, &dispatched) !=
            DM2_V1_SOURCE_TIMER_OK ||
        dispatched != 3U || receipt.count != 3U ||
        receipt.types[0] != 4U || receipt.types[1] != 2U ||
        receipt.types[2] != 2U || queue.count != 1U) {
        return 1;
    }
    if (strstr(dm2_v1_source_timer_source_evidence(), "c_tim_proc.cpp") ==
        NULL) {
        return 1;
    }
    puts("DM2 raw c_tim queue receipt PASS");
    return 0;
}
