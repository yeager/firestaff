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

static inline uint32_t dm2_v1_source_timer_tick(const DM2_V1_SourceTimer *t) {
    return t->ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK;
}

typedef struct {
    DM2_V1_SourceTimer timers[DM2_V1_SOURCE_TIMER_MAX];
    uint16_t source_indices[DM2_V1_SOURCE_TIMER_MAX];
    /* Stable session-issued timer identities (additive DM2-003
     * follow-up): mirrors the source timerarray slot index —
     * c_timer.cpp:235-257 DM2_QUEUE_TIMER returns the stable slot index
     * from the available_timeridx free-list, and c_timer.cpp:215-232
     * DM2_DELETE_TIMER clears the slot and pushes it back.  Ticket 0 is
     * "no timer" (the source stores -1 in owner words like the CAII
     * slot timer word; bounded slices keep 0 to stay unsigned). */
    uint32_t tickets[DM2_V1_SOURCE_TIMER_MAX];
    uint32_t next_ticket;
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
/* Ticketed enqueue (DM2-003 follow-up): identical ordering to
 * dm2_v1_source_timer_enqueue but issues the stable session ticket the
 * source returns from DM2_QUEUE_TIMER (c_timer.cpp:235-257).  Returns
 * the ticket (> 0) on success, 0 on rejection/full; when `out_result`
 * is non-NULL it receives the legacy result code. */
uint32_t dm2_v1_source_timer_enqueue_ticketed(
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_SourceTimerResult *out_result);
/* Cancel the timer carrying `ticket` (source DM2_DELETE_TIMER slice,
 * c_timer.cpp:215-232): removes it from the queue, 1 when found and
 * removed, 0 when the ticket is 0 or unknown (fail-closed). */
int dm2_v1_source_timer_cancel(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t ticket);
/* Read-only lookup of the timer carrying `ticket` (source timerarray
 * slot read, e.g. c_1c9a.cpp:5943-5944 reading the xA/yA payload bytes
 * before DM2_DELETE_TIMER runs).  Copies the queued timer to
 * `out_timer` and returns 1 when the ticket is live; returns 0 for
 * ticket 0, unknown tickets, or NULL out-param (fail-closed, no
 * mutation). */
int dm2_v1_source_timer_peek_ticket(
    const DM2_V1_SourceTimerQueue *queue,
    uint32_t ticket,
    DM2_V1_SourceTimer *out_timer);
/* In-place payload update of the timer carrying `ticket` (source
 * c_moverec.cpp:977-978 inside DM2_moverec_3CE7D:
 * timdat.timerarray[idx].setxyA(x, y) + setmticks(map, getticks()) —
 * c_timer.h:82 setxyA writes the valueA lo/hi bytes, c_timer.h:66
 * setmticks rewrites l_00 = (map << 24) | the preserved ticks).  The
 * queue order is untouched: the source updates the timerarray slot in
 * place and every DM2_cmp_timers key (ticks, type, actor, index) is
 * preserved.  Returns 1 when the ticket is live and the timer was
 * updated, 0 for ticket 0 or an unknown ticket (fail-closed, no
 * mutation). */
int dm2_v1_source_timer_update_payload(DM2_V1_SourceTimerQueue *queue,
                                       uint32_t ticket,
                                       int x, int y, int map);
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
