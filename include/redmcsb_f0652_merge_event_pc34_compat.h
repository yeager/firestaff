#ifndef REDMCSB_F0652_MERGE_EVENT_PC34_COMPAT_H
#define REDMCSB_F0652_MERGE_EVENT_PC34_COMPAT_H

#include "dm1_v1_event_timer_pc34_compat.h"

/* ReDMCSB TIMELINE.C F0652_MergeEvent.
 *
 * This operates on Firestaff's existing native EVENT/TIMELINE owner. It
 * performs only the source merge and F0237 deletion transaction: C05..C10
 * map events, C01 door animation, and C02 door destruction. It does not add
 * a new event, dispatch an event, interpret a save layout, or execute DSA. */

enum {
    REDMCSB_F0652_PC34_NO_MERGE = -1,
    REDMCSB_F0652_PC34_INVALID_QUEUE = -2
};

/* largest_used_event_ordinal is F0651's source M000 ordinal, not an index.
 * Returns an existing event index for a source merge; otherwise -1. */
int redmcsb_f0652_merge_event_pc34(
    struct DM1_EventQueue_V1 *queue, int largest_used_event_ordinal,
    struct DM1_Event_V1 *incoming_event);

const char *redmcsb_f0652_merge_event_pc34_source_evidence(void);

#endif
