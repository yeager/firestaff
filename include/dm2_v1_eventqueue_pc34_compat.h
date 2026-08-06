#ifndef FIRESTAFF_DM2_V1_EVENTQUEUE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_EVENTQUEUE_PC34_COMPAT_H

/*
 * dm2_v1_eventqueue_pc34_compat.h — DM2 event queue management.
 *
 * Ports c_eventqueue from skproject c_eventqueue.cpp: mouse/key event
 * circular queue with semaphore-based single-event buffering.
 *
 * Source: skproject/SKWINSPX/src/v4/c_eventqueue.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_EVENTQUEUE_LEN 11

typedef struct {
    int16_t x, y, b;
} DM2_V1_EventEntry;

typedef struct {
    int16_t idx, out_idx, entries;
    bool fetch_busy;
    /* c_eventqueue.cpp::QUEUE_EVENT keeps this static edge across calls:
     * a saturated 0x02 event changes only the following 0x04 capacity. */
    bool button0x2;
    DM2_V1_EventEntry data[DM2_V1_EVENTQUEUE_LEN];
    bool singleevent_available;
    DM2_V1_EventEntry singleevent;
    int16_t event_unk02, event_unk03, event_unk04, event_unk05;
    int16_t event_unk06, event_unk07, event_unk08, event_unk09;
    int16_t event_unk0a;
    int16_t event_heroidx;
    bool event_unk0f;
} DM2_V1_EventQueue;

typedef struct {
    bool queued;
    int16_t slot;
} DM2_V1_QueueEventReceipt;

typedef struct {
    bool processed;
    bool had_event;
} DM2_V1_ProcessSingleEventReceipt;

void dm2_v1_eventqueue_init(DM2_V1_EventQueue *eq);
void dm2_v1_eventqueue_set(DM2_V1_EventQueue *eq, int16_t i,
                           int16_t x, int16_t y, int16_t b);
DM2_V1_ProcessSingleEventReceipt dm2_v1_eventqueue_process_singleevent(
    DM2_V1_EventQueue *eq);
DM2_V1_QueueEventReceipt dm2_v1_eventqueue_queue_event(
    DM2_V1_EventQueue *eq, int16_t mx, int16_t my, int16_t mb);
DM2_V1_QueueEventReceipt dm2_v1_eventqueue_queue_0x20(
    DM2_V1_EventQueue *eq, int16_t key);
void dm2_v1_eventqueue_flush(DM2_V1_EventQueue *eq);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_EVENTQUEUE_PC34_COMPAT_H */
