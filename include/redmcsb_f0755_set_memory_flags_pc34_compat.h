/*
 * ReDMCSB STARTUP2.C F0755_SetMemoryFlags, PC 3.4 route.
 *
 * STARTUP2.C:1054-1079 consumes one sound-memory profile. It clears six
 * memory flags, recognizes the FD size marker, and stops at FE or FF.
 */
#ifndef FIRESTAFF_REDMCSB_F0755_SET_MEMORY_FLAGS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0755_SET_MEMORY_FLAGS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0755_MEMORY_FLAG_COUNT_PC34_COMPAT = 6
};

/*
 * Applies F0755 to one valid profile stream. Like the source, this routine
 * assumes that every ordinary byte indexes the six-byte flag array and that
 * FD has one following byte. It deliberately performs no malformed-stream
 * recovery.
 */
int redmcsb_f0755_set_memory_flags_pc34_compat(
    uint8_t **profile_cursor,
    int32_t *byte_count,
    uint8_t memory_flags[REDMCSB_F0755_MEMORY_FLAG_COUNT_PC34_COMPAT]);

const char *redmcsb_f0755_set_memory_flags_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
