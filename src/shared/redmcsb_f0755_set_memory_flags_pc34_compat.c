#include "redmcsb_f0755_set_memory_flags_pc34_compat.h"

int redmcsb_f0755_set_memory_flags_pc34_compat(
    uint8_t **profile_cursor,
    int32_t *byte_count,
    uint8_t memory_flags[REDMCSB_F0755_MEMORY_FLAG_COUNT_PC34_COMPAT])
{
    uint8_t *cursor;
    uint8_t counter;
    uint8_t index;

    /* ReDMCSB STARTUP2.C:1062-1078, MEDIA728 PC 3.4 route. */
    cursor = *profile_cursor;
    for (index = 0; index < REDMCSB_F0755_MEMORY_FLAG_COUNT_PC34_COMPAT;
         index++) {
        memory_flags[index] = 0U;
    }
    *byte_count = 0;
    while ((counter = *cursor++) != 0xFEU) {
        if (counter == 0xFFU) {
            return 0;
        }
        if (counter == 0xFDU) {
            *byte_count = (int32_t)(*cursor++) * 1024;
        } else {
            memory_flags[counter] = 1U;
        }
    }
    *profile_cursor = cursor;
    return *cursor != 0xFFU;
}

const char *redmcsb_f0755_set_memory_flags_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 STARTUP2.C:1054-1079 defines "
           "F0755_SetMemoryFlags: clear G2040's six flags, scan the profile "
           "until FE, use FD's next byte as a KiB count, preserve the cursor "
           "after FE, and reject FF either before or after that sentinel.";
}
