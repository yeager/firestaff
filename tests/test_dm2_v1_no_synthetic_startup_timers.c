#include "dm2_v1_timeline.h"

#include <stdio.h>

static int unexpected_dispatches;

static bool reject_unexpected_timer(void *context,
                                    const DM2_V1_SourceTimer *timer,
                                    uint16_t source_index)
{
    (void)context;
    (void)timer;
    (void)source_index;
    ++unexpected_dispatches;
    return false;
}

int main(void) {
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer timer;
    uint16_t source_index;
    size_t dispatched = 99U;

    /* SKProject c_timer.cpp only queues timers supplied by the loaded
     * dungeon/session.  A presentation-only boot has no source timer rows,
     * so it must never manufacture a startup, credits, or menu event. */
    dm2_v1_source_timer_queue_init(&queue);
    if (queue.count != 0U ||
        dm2_v1_source_timer_is_due(&queue, 60000U) ||
        dm2_v1_source_timer_pop_due(&queue, 60000U, &timer, &source_index) !=
            DM2_V1_SOURCE_TIMER_EMPTY ||
        dm2_v1_source_timer_dispatch_due(&queue, 60000U,
                                         reject_unexpected_timer, NULL,
                                         &dispatched) != DM2_V1_SOURCE_TIMER_OK ||
        dispatched != 0U || unexpected_dispatches != 0) {
        return 1;
    }

    puts("PASS: DM2 startup creates no synthetic source timers");
    return 0;
}
