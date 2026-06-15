#ifndef CSB_V1_QUEUE_OVERFLOW_PC34_COMPAT_H
#define CSB_V1_QUEUE_OVERFLOW_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_QUEUE_OVERFLOW_STORAGE_SIZE_PC34 = 8,
    CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34 = 5,
    CSB_V1_QUEUE_OVERFLOW_RESERVED_CAP_PC34 = 7,
    CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34 = 12
};

static inline const char *csb_v1_queue_overflow_source_evidence_pc34_compat(void)
{
    /* ReDMCSB source-lock:
     * DEFS.H lines 3261-3264 selects M529_COMMAND_QUEUE_SIZE 8 for the
     * I34E/I34M/A35 build family used by CSB PC-34 compatibility.
     * DEFS.H lines 3507-3509 defines C5 and C7 queue-count limits.
     * COMMAND.C lines 6-11 declares the circular command queue globals.
     * COMMAND.C F0359 lines 1506-1514 caps regular mouse input at C5 and
     * reserves C7 for release/stop events.
     * COMMAND.C F0361 lines 1744-1766 applies the same C5 cap to keyboard
     * command enqueue.
     * COMMAND.C F0380 lines 2075-2126 dequeues one queued command and wraps
     * the first index across M529_COMMAND_QUEUE_SIZE.
     */
    return "DEFS.H:3261-3264 M529_COMMAND_QUEUE_SIZE=8 for I34/CSB-compatible builds; "
           "DEFS.H:3507-3509 C5/C7 queue caps; "
           "COMMAND.C:6-11 queue globals; "
           "COMMAND.C F0359:1506-1514 regular mouse cap/reserved release slots; "
           "COMMAND.C F0361:1744-1766 keyboard C5 cap; "
           "COMMAND.C F0380:2075-2126 dequeue and wrap.";
}

#ifdef __cplusplus
}
#endif

#endif
