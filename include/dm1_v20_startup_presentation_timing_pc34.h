#ifndef FIRESTAFF_DM1_V20_STARTUP_PRESENTATION_TIMING_PC34_H
#define FIRESTAFF_DM1_V20_STARTUP_PRESENTATION_TIMING_PC34_H

#include <stdint.h>

/* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0438 schedule presentation at
 * source VBlank boundaries. V2.0 filtering is host-side work performed while
 * committing the already source-owned indexed frame, so it consumes part of
 * that boundary rather than adding a second delay after it. */
uint32_t dm1_v20_startup_presentation_remaining_delay_ms_pc34(
    uint32_t source_delay_ms,
    uint64_t presentation_elapsed_ms);

#endif
