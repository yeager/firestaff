#ifndef FIRESTAFF_DM2_V1_INVOKE_MESSAGE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_INVOKE_MESSAGE_PC34_COMPAT_H

/*
 * dm2_v1_invoke_message_pc34_compat.h — DM2_INVOKE_MESSAGE
 * (skproject/SKULLWIN/c_tim_proc.cpp:4332-4367) as a bounded slice over
 * the source-owned timer queue.
 *
 * The source builds a c_tim and queues it (DM2_QUEUE_TIMER,
 * c_timer.cpp:235-257):
 *
 *   c_tim_proc.cpp:4338  setmticks(ddat.v1d3248, argl0) — c_timer.h:66:
 *                        ticks_and_map = (map << 24) | tick (the source
 *                        ORs the tick unmasked; the bounded slice masks
 *                        it with DM2_V1_SOURCE_TIMER_TICK_MASK exactly
 *                        like the proven scheduling producer, since the
 *                        tick owner keeps gametick < 2^24);
 *   c_tim_proc.cpp:4339  settype(0x4);
 *   c_tim_proc.cpp:4340-4357  actor from ecxl (RG3UW): 0 -> 1, 1 -> 3,
 *                        2 -> 2, anything else keeps the c_tim init
 *                        default 0;
 *   c_tim_proc.cpp:4359  setxyA(eaxl lo, edxl lo) — c_timer.h:82:
 *                        value_a = (ya & 0xff) << 8 | (xa & 0xff);
 *   c_tim_proc.cpp:4360  setxyB(ebxl lo, ecxl lo) — c_timer.h:90:
 *                        value_b = (sel & 0xff) << 8 | (xb & 0xff);
 *   c_tim_proc.cpp:4361  DM2_QUEUE_TIMER issues the stable session
 *                        ticket (the enqueue is bound ticketed with
 *                        source_index 0u, the producer-module
 *                        convention).
 *
 * Callers bind their own call sites: DM2_DELETE_CREATURE_RECORD
 * (c_record.cpp:1397-1405) invokes with (x, y, 0, 0, gametick + 1)
 * after the map swap; the c_tim_proc.cpp:1246 + 4391 sites stay with
 * their owners.  What a queued type-0x4 timer DOES at dispatch
 * (DM2_PROCEED_TIMERS' message handling) stays host-owned — this slice
 * binds only the timer construction and the queue.
 *
 * Fail-closed contract: a rejected enqueue (queue full) is receipted
 * queue_rejected and returns 0 without any further mutation; NULL
 * queue or an out-of-range map byte fails closed.
 */

#include <stdint.h>

#include "dm2_v1_timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int type;                 /* always 0x4 (c_tim_proc.cpp:4339) */
    int actor;                /* 0/1/2/3 per the RG3UW mapping */
    int16_t value_a;          /* setxyA payload */
    int16_t value_b;          /* setxyB payload */
    uint32_t ticks_and_map;   /* setmticks word */
    uint32_t ticket;          /* stable session ticket (> 0 when queued) */
    int queue_rejected;       /* fail-closed: DM2_QUEUE_TIMER rejected */
    char source_evidence[512];
} DM2_V1_InvokeMessageReceipt;

/* DM2_INVOKE_MESSAGE bounded slice.  `map_current` stands in for
 * ddat.v1d3248 at the call site (after any caller-owned map swap);
 * `xa`/`ya` are eaxl/edxl, `xb`/`sel` are ebxl/ecxl, `tick` is argl0.
 * Returns 1 when the timer was queued (receipt.valid == 1,
 * receipt.ticket > 0); 0 (fail-closed, receipted) otherwise.  When
 * `receipt` is non-NULL it always receives the audit record. */
int dm2_v1_invoke_message(
    DM2_V1_SourceTimerQueue *queue,
    int map_current,
    int xa, int ya,
    int xb, int sel,
    int32_t tick,
    DM2_V1_InvokeMessageReceipt *receipt);

const char *dm2_v1_invoke_message_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INVOKE_MESSAGE_PC34_COMPAT_H */
