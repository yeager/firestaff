#include "redmcsb_f0651_timeline_free_list_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void set_event_type(uint8_t *events, size_t stride, size_t index,
                           uint8_t type)
{
    events[index * stride + REDMCSB_F0651_PC34_EVENT_TYPE_OFFSET] = type;
}

int main(void)
{
    uint8_t events[6U * 10U];
    uint8_t active_before[10];
    RedmcsbF0651TimelineManagementReceiptPc34 receipt;
    int result;

    memset(events, 0xA5, sizeof(events));
    set_event_type(events, 10U, 0U, 1U);
    set_event_type(events, 10U, 1U, REDMCSB_F0651_PC34_EVENT_NONE);
    set_event_type(events, 10U, 2U, 2U);
    set_event_type(events, 10U, 3U, REDMCSB_F0651_PC34_EVENT_NONE);
    set_event_type(events, 10U, 4U, REDMCSB_F0651_PC34_EVENT_NONE);
    set_event_type(events, 10U, 5U, 3U);
    events[10U] = 0x9AU;
    events[11U] = 0xBCU;
    events[30U] = 0xDEU;
    events[31U] = 0xF0U;
    events[40U] = 0x12U;
    events[41U] = 0x34U;
    memcpy(active_before, events + 20U, sizeof(active_before));

    result = redmcsb_f0651_rebuild_unused_event_list_pc34(
        events, 6U, 10U, &receipt);
    CHECK("F0651 accepts caller-admitted raw EVENT records",
          result == REDMCSB_F0651_PC34_RESULT_OK);
    CHECK("F0651 derives first unused and largest used ordinal",
          receipt.first_unused_event_index == 1 &&
          receipt.largest_used_event_ordinal == 6 &&
          receipt.unused_event_count == 3U && receipt.used_event_count == 3U);
    CHECK("F0651 overwrites stale serialized UNUSED_EVENT links in scan order",
          read_le16(events + 10U) == 3U && read_le16(events + 30U) == 4U &&
          read_le16(events + 40U) == UINT16_MAX);
    CHECK("F0651 leaves active EVENT bytes untouched",
          memcmp(events + 20U, active_before, sizeof(active_before)) == 0);

    memset(events, 0, sizeof(events));
    set_event_type(events, 10U, 0U, 4U);
    set_event_type(events, 10U, 1U, 5U);
    CHECK("F0651 records no free-list when no EVENT_NONE exists",
          redmcsb_f0651_rebuild_unused_event_list_pc34(events, 2U, 10U,
                                                        &receipt) ==
              REDMCSB_F0651_PC34_RESULT_OK &&
          receipt.first_unused_event_index == -1 &&
          receipt.largest_used_event_ordinal == 2 &&
          receipt.unused_event_count == 0U && receipt.used_event_count == 2U);
    CHECK("F0651 accepts an empty admitted EVENT array without a replacement",
          redmcsb_f0651_rebuild_unused_event_list_pc34(NULL, 0U, 10U,
                                                        &receipt) ==
              REDMCSB_F0651_PC34_RESULT_OK &&
          receipt.first_unused_event_index == -1 &&
          receipt.largest_used_event_ordinal == 0);
    CHECK("F0651 rejects an unproven event stride",
          redmcsb_f0651_rebuild_unused_event_list_pc34(events, 1U, 4U,
                                                        &receipt) ==
              REDMCSB_F0651_PC34_RESULT_PRECONDITION_FAILED);
    CHECK("source evidence is available",
          strstr(redmcsb_f0651_timeline_free_list_pc34_source_evidence(),
                 "F0651") != NULL);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB F0651 timeline free-list rebuild");
    return 0;
}
