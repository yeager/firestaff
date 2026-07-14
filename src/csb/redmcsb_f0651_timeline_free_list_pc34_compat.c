#include "redmcsb_f0651_timeline_free_list_pc34_compat.h"

#include <limits.h>

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

int redmcsb_f0651_rebuild_unused_event_list_pc34(
    uint8_t *events, size_t event_count, size_t event_stride,
    RedmcsbF0651TimelineManagementReceiptPc34 *receipt)
{
    int16_t previous_unused = -1;
    size_t event_index;

    if (receipt != NULL) {
        *receipt = (RedmcsbF0651TimelineManagementReceiptPc34){
            -1, 0, 0U, 0U
        };
    }
    if (receipt == NULL || event_count > (size_t)INT16_MAX ||
        event_stride < REDMCSB_F0651_PC34_EVENT_TYPE_OFFSET + 1U ||
        (event_count != 0U && events == NULL) ||
        (event_count != 0U && event_stride > SIZE_MAX / event_count)) {
        return REDMCSB_F0651_PC34_RESULT_PRECONDITION_FAILED;
    }

    for (event_index = 0U; event_index < event_count; ++event_index) {
        uint8_t *event = events + event_index * event_stride;

        if (event[REDMCSB_F0651_PC34_EVENT_TYPE_OFFSET] ==
            REDMCSB_F0651_PC34_EVENT_NONE) {
            if (receipt->first_unused_event_index == -1) {
                receipt->first_unused_event_index = (int16_t)event_index;
            } else {
                uint8_t *previous = events +
                    (size_t)previous_unused * event_stride;

                write_le16(previous + REDMCSB_F0651_PC34_UNUSED_NEXT_OFFSET,
                           (uint16_t)event_index);
            }
            write_le16(event + REDMCSB_F0651_PC34_UNUSED_NEXT_OFFSET,
                       UINT16_MAX);
            previous_unused = (int16_t)event_index;
            ++receipt->unused_event_count;
            continue;
        }
        receipt->largest_used_event_ordinal = (int16_t)(event_index + 1U);
        ++receipt->used_event_count;
    }
    return REDMCSB_F0651_PC34_RESULT_OK;
}

const char *redmcsb_f0651_timeline_free_list_pc34_source_evidence(void)
{
    return "ReDMCSB TIMELINE.C F0651_TIMELINE_InitializeOptimizedManagement "
           "(lines 100-124); DEFS.H EVENT and UNUSED_EVENT";
}
