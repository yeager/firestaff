/*
 * dm2_v1_eventqueue_pc34_compat.c — DM2 event queue management.
 *
 * Source: skproject/SKWINSPX/src/v4/c_eventqueue.cpp
 */

#include "dm2_v1_eventqueue_pc34_compat.h"
#include <string.h>

void dm2_v1_eventqueue_init(DM2_V1_EventQueue *eq)
{
    memset(eq, 0, sizeof(*eq));
    eq->event_heroidx = 0;
    eq->event_unk05 = -1;
    eq->event_unk09 = -1;
}

void dm2_v1_eventqueue_set(DM2_V1_EventQueue *eq, int16_t i,
                           int16_t x, int16_t y, int16_t b)
{
    if (i < 0 || i >= DM2_V1_EVENTQUEUE_LEN)
        return;
    eq->idx = i;
    eq->data[i].x = x;
    eq->data[i].y = y;
    eq->data[i].b = b;
    eq->entries++;
}

DM2_V1_ProcessSingleEventReceipt dm2_v1_eventqueue_process_singleevent(
    DM2_V1_EventQueue *eq)
{
    DM2_V1_ProcessSingleEventReceipt r = {false, false};

    if (!eq->singleevent_available)
        return r;

    eq->singleevent_available = false;
    r.had_event = true;

    /* Queue the buffered single event */
    DM2_V1_QueueEventReceipt qr = dm2_v1_eventqueue_queue_event(
        eq, eq->singleevent.x, eq->singleevent.y, eq->singleevent.b);
    r.processed = qr.queued;
    return r;
}

DM2_V1_QueueEventReceipt dm2_v1_eventqueue_queue_event(
    DM2_V1_EventQueue *eq, int16_t mx, int16_t my, int16_t mb)
{
    DM2_V1_QueueEventReceipt r = {false, -1};

    if (eq->fetch_busy) {
        /* Store as single event for later processing */
        eq->singleevent.x = mx;
        eq->singleevent.y = my;
        eq->singleevent.b = mb;
        eq->singleevent_available = true;
        return r;
    }

    /* Capacity depends on button type */
    int16_t cap = 7;
    if (mb == 0x04 || mb == 0x40 || mb == 0x60)
        cap = 9;

    if (eq->entries >= cap)
        return r;

    /* Insert at next slot after idx, wrapping */
    int16_t slot = (int16_t)((eq->idx + 1) % DM2_V1_EVENTQUEUE_LEN);
    eq->data[slot].x = mx;
    eq->data[slot].y = my;
    eq->data[slot].b = mb;
    eq->idx = slot;
    eq->entries++;
    r.queued = true;
    r.slot = slot;
    return r;
}

DM2_V1_QueueEventReceipt dm2_v1_eventqueue_queue_0x20(
    DM2_V1_EventQueue *eq, int16_t key)
{
    eq->fetch_busy = true;

    /* Insert keyboard event with b=0x20 */
    int16_t slot = (int16_t)((eq->idx + 1) % DM2_V1_EVENTQUEUE_LEN);
    eq->data[slot].x = key;
    eq->data[slot].y = 0;
    eq->data[slot].b = 0x20;
    eq->idx = slot;
    eq->entries++;

    DM2_V1_QueueEventReceipt r;
    r.queued = true;
    r.slot = slot;

    eq->fetch_busy = false;

    /* Process any buffered single event */
    dm2_v1_eventqueue_process_singleevent(eq);

    return r;
}

void dm2_v1_eventqueue_flush(DM2_V1_EventQueue *eq)
{
    /* Compact queue: keep only entries with b == 0x04, 0x40, or 0x60 */
    DM2_V1_EventEntry kept[DM2_V1_EVENTQUEUE_LEN];
    int16_t kept_count = 0;

    /* Scan all slots in the queue.  Events are inserted at idx+1 wrapping,
     * so the oldest entry is at (idx - entries + 1) mod LEN when out_idx
     * has not been advanced.  Scan the entire ring to be safe. */
    int16_t start = (int16_t)((eq->idx - eq->entries + 1 + DM2_V1_EVENTQUEUE_LEN)
                              % DM2_V1_EVENTQUEUE_LEN);
    for (int16_t i = 0; i < eq->entries && i < DM2_V1_EVENTQUEUE_LEN; i++) {
        int16_t pos = (int16_t)((start + i) % DM2_V1_EVENTQUEUE_LEN);
        int16_t b = eq->data[pos].b;
        if (b == 0x04 || b == 0x40 || b == 0x60) {
            kept[kept_count++] = eq->data[pos];
        }
    }

    /* Rewrite queue from slot 0 */
    for (int16_t i = 0; i < kept_count; i++)
        eq->data[i] = kept[i];

    eq->out_idx = 0;
    eq->idx = (int16_t)(kept_count > 0 ? kept_count - 1 : 0);
    eq->entries = kept_count;

    /* Reset event_unk fields */
    eq->event_unk02 = 0;
    eq->event_unk03 = 0;
    eq->event_unk04 = 0;
    eq->event_unk05 = -1;
    eq->event_unk06 = 0;
    eq->event_unk07 = 0;
    eq->event_unk08 = 0;
    eq->event_unk09 = -1;
    eq->event_unk0a = 0;
    eq->event_unk0f = false;
}
