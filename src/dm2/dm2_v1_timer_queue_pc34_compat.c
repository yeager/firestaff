#include "dm2_v1_timer_queue_pc34_compat.h"
#include <string.h>

/* Source: sktimer.cpp DM2_cmp_timers
 * Returns true if a should come before b in the min-heap. */
static int cmp_timers(const DM2_V1_TimerEntry *a, const DM2_V1_TimerEntry *b)
{
    int32_t ta = dm2_v1_timer_get_ticks(a);
    int32_t tb = dm2_v1_timer_get_ticks(b);
    if (ta < tb) return 1;
    if (ta != tb) return 0;
    if (a->ttype > b->ttype) return 1;
    if (a->ttype != b->ttype) return 0;
    if (a->actor > b->actor) return 1;
    if (a->actor != b->actor) return 0;
    if (a > b) return 0;
    return 1;
}

/* Source: sktimer.cpp DM2_timer_3a15_0486
 * Combined bubble-up / bubble-down sift for heap position pos. */
static void sift(DM2_V1_TimerQueue *q, int16_t pos)
{
    q->deferred_sift = -1;

    int16_t last = q->num_timers - 1;
    if (last == 0)
        return;

    int16_t cur = pos;
    int16_t saved_slot = q->indices[cur];
    DM2_V1_TimerEntry *saved = &q->entries[saved_slot];
    int bubbled_up = 0;

    /* Bubble up */
    while (cur != 0) {
        int16_t parent = (cur - 1) / 2;
        if (!cmp_timers(saved, &q->entries[q->indices[parent]]))
            break;
        q->indices[cur] = q->indices[parent];
        cur = parent;
        bubbled_up = 1;
    }

    /* Bubble down (only if we didn't bubble up) */
    if (!bubbled_up) {
        int16_t half = (last - 1) / 2;
        while (cur <= half) {
            int16_t child = 2 * cur + 1;
            if ((child + 1) < q->num_timers
                && cmp_timers(&q->entries[q->indices[child + 1]],
                              &q->entries[q->indices[child]]))
                child++;
            if (!cmp_timers(&q->entries[q->indices[child]], saved))
                break;
            q->indices[cur] = q->indices[child];
            cur = child;
        }
    }

    q->indices[cur] = saved_slot;
}

/* Source: sktimer.cpp DM2_REARRANGE_TIMERLIST */
static void rearrange_freelist(DM2_V1_TimerQueue *q)
{
    int16_t last_free = -1;
    q->num_indices = 0;
    q->available_idx = -1;

    for (int16_t n = 0; n < q->max_timers; n++) {
        if (q->entries[n].ttype != 0) {
            q->num_indices = n + 1;
        } else {
            if (q->available_idx != -1) {
                q->entries[last_free].l_00 =
                    (q->entries[last_free].l_00 & ~0xffff) | (uint16_t)n;
            } else {
                q->available_idx = n;
            }
            q->entries[n].l_00 =
                (q->entries[n].l_00 & ~0xffff) | (uint16_t)(-1 & 0xffff);
            last_free = n;
        }
    }
}

void dm2_v1_timer_entry_init(DM2_V1_TimerEntry *t)
{
    memset(t, 0, sizeof(*t));
}

void dm2_v1_timer_queue_init(DM2_V1_TimerQueue *q,
                             DM2_V1_TimerEntry *entries,
                             int16_t *indices,
                             int16_t max_timers)
{
    q->entries = entries;
    q->indices = indices;
    q->num_indices = 0;
    q->num_timers = 0;
    q->available_idx = 0;
    q->max_timers = max_timers;
    q->deferred_sift = -1;
    q->gametick = 0;

    for (int16_t i = 0; i < max_timers; i++) {
        dm2_v1_timer_entry_init(&entries[i]);
        int16_t next = (i + 1 < max_timers) ? (i + 1) : -1;
        entries[i].l_00 = (int32_t)(uint16_t)(next & 0xffff);
    }
}

/* Source: sktimer.cpp DM2_SORT_TIMERS */
void dm2_v1_timer_sort(DM2_V1_TimerQueue *q)
{
    if (q->num_timers != 0) {
        int16_t n_timers = q->num_timers;

        /* Phase 1: assign indices 0..n-1 to the first n timer slots */
        for (int16_t i = 0; i < n_timers; i++)
            q->indices[i] = i;

        /* Phase 2: heapify (bottom-up) */
        if (n_timers != 1) {
            int16_t start = (n_timers - 2) / 2;
            for (int16_t i = start; i >= 0; i--) {
                int16_t pos = i;
                DM2_V1_TimerEntry *pivot = &q->entries[q->indices[i]];

                for (;;) {
                    int16_t child = 2 * pos + 1;
                    if (child >= q->num_timers)
                        break;

                    DM2_V1_TimerEntry *left = &q->entries[q->indices[child]];
                    if ((child + 1) >= q->num_timers) {
                        if (cmp_timers(pivot, left))
                            break;
                    } else {
                        DM2_V1_TimerEntry *right = &q->entries[q->indices[child + 1]];
                        if (cmp_timers(pivot, left)) {
                            if (cmp_timers(pivot, right))
                                break;
                            child = child + 1;
                        } else {
                            if (cmp_timers(right, left))
                                child = child + 1;
                        }
                    }

                    int16_t tmp = q->indices[pos];
                    q->indices[pos] = q->indices[child];
                    q->indices[child] = tmp;
                    pos = child;
                }
            }
        }
    }

    q->deferred_sift = -1;
    rearrange_freelist(q);
}

/* Source: sktimer.cpp DM2_GET_TIMER_NEW_INDEX */
int16_t dm2_v1_timer_get_heap_index(const DM2_V1_TimerQueue *q, int16_t slot)
{
    for (int16_t n = 0; n < q->num_indices; n++) {
        if (q->indices[n] == slot)
            return n;
    }
    return -1;
}

/* Source: sktimer.cpp DM2_DELETE_TIMER */
void dm2_v1_timer_delete(DM2_V1_TimerQueue *q, int16_t slot)
{
    if (q->deferred_sift >= 0)
        sift(q, q->deferred_sift);

    q->entries[slot].ttype = 0;
    q->entries[slot].l_00 =
        (q->entries[slot].l_00 & ~0xffff) | (uint16_t)(q->available_idx & 0xffff);
    q->available_idx = slot;
    q->num_timers--;

    if (q->num_timers != -1) {
        int16_t heap_pos = dm2_v1_timer_get_heap_index(q, slot);
        if (heap_pos >= 0 && heap_pos != q->num_timers) {
            q->indices[heap_pos] = q->indices[q->num_timers];
            q->deferred_sift = heap_pos;
        }
    }
}

/* Source: sktimer.cpp DM2_QUEUE_TIMER */
int16_t dm2_v1_timer_queue(DM2_V1_TimerQueue *q, const DM2_V1_TimerEntry *timer)
{
    if (timer->ttype == 0)
        return -1;

    if (q->num_timers == q->max_timers)
        return -1;

    int16_t slot = q->available_idx;
    DM2_V1_TimerEntry *dest = &q->entries[slot];
    q->available_idx = (int16_t)(dest->l_00 & 0xffff);
    *dest = *timer;

    if (slot >= q->num_indices)
        q->num_indices = slot + 1;

    int16_t heap_pos = q->deferred_sift;
    if (heap_pos < 0)
        heap_pos = q->num_timers;
    q->deferred_sift = -1;
    q->num_timers++;
    q->indices[heap_pos] = slot;
    sift(q, heap_pos);
    return slot;
}

/* Source: sktimer.cpp DM2_GET_AND_DELETE_NEXT_TIMER */
void dm2_v1_timer_get_and_delete_next(DM2_V1_TimerQueue *q, DM2_V1_TimerEntry *out)
{
    if (q->deferred_sift >= 0)
        sift(q, q->deferred_sift);
    int16_t slot = q->indices[0];
    *out = q->entries[slot];
    dm2_v1_timer_delete(q, slot);
}

/* Source: sktimer.cpp DM2_IS_TIMER_TO_PROCEED */
int dm2_v1_timer_is_due(DM2_V1_TimerQueue *q)
{
    if (q->deferred_sift >= 0)
        sift(q, q->deferred_sift);

    if (q->num_timers == 0)
        return 0;

    uint32_t tick = (uint32_t)dm2_v1_timer_get_ticks(&q->entries[q->indices[0]]);
    return tick <= (uint32_t)q->gametick;
}

/* Source: sktimer.cpp DM2_timer_3a15_05f7 */
void dm2_v1_timer_reheapify(DM2_V1_TimerQueue *q, int16_t slot)
{
    if (q->deferred_sift >= 0)
        sift(q, q->deferred_sift);

    int16_t heap_pos = dm2_v1_timer_get_heap_index(q, slot);
    if (heap_pos >= 0)
        sift(q, heap_pos);
}
