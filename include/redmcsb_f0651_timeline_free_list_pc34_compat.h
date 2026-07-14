#ifndef REDMCSB_F0651_TIMELINE_FREE_LIST_PC34_COMPAT_H
#define REDMCSB_F0651_TIMELINE_FREE_LIST_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB TIMELINE.C F0651, the post-F0435 EVENT management rebuild.
 *
 * The original scans every loaded EVENT. EVENT.Type is byte 4 after the
 * source Map_Time word; an EVENT_NONE record is overlaid as UNUSED_EVENT and
 * receives a new little-endian NextUnusedEventIndex at bytes 0..1. Existing
 * serialized links are intentionally overwritten. The caller supplies the
 * authenticated media-specific EVENT stride; this routine executes no event,
 * interprets no payload, and does not rebuild the C4 heap. */

#define REDMCSB_F0651_PC34_EVENT_TYPE_OFFSET 4U
#define REDMCSB_F0651_PC34_UNUSED_NEXT_OFFSET 0U
#define REDMCSB_F0651_PC34_EVENT_NONE 0U

enum {
    REDMCSB_F0651_PC34_RESULT_OK = 1,
    REDMCSB_F0651_PC34_RESULT_PRECONDITION_FAILED = 0
};

typedef struct {
    int16_t first_unused_event_index;
    int16_t largest_used_event_ordinal; /* M000_INDEX_TO_ORDINAL: index + 1 */
    unsigned int unused_event_count;
    unsigned int used_event_count;
} RedmcsbF0651TimelineManagementReceiptPc34;

/* event_count is the already-admitted GLOBAL_DATA.EventMaximumCount. */
int redmcsb_f0651_rebuild_unused_event_list_pc34(
    uint8_t *events, size_t event_count, size_t event_stride,
    RedmcsbF0651TimelineManagementReceiptPc34 *receipt);

const char *redmcsb_f0651_timeline_free_list_pc34_source_evidence(void);

#endif
