#include "redmcsb_f0789_allocate_layout_range_pc34_compat.h"

unsigned char *redmcsb_f0789_allocate_layout_range_pc34_compat(
    unsigned long byte_count,
    redmcsb_f0789_memory_allocate_pc34_compat_fn memory_allocate)
{
    if (memory_allocate == 0) {
        return 0;
    }

    return memory_allocate(byte_count,
                           REDMCSB_F0789_ALLOCATION_PERMANENT,
                           REDMCSB_F0789_MEMORY_REQUEST_LAYOUT_RANGE);
}

const char *redmcsb_f0789_allocate_layout_range_source_evidence_pc34(void)
{
    return "ReDMCSB COORD.C:2536-2539 (PC 3.4 I34E/I34M route): "
           "F0789_AllocateLayoutRange forwards P2150_puc_ByteCount to "
           "F0468_MEMORY_Allocate with C1_ALLOCATION_PERMANENT and "
           "MASK0x0400_MEMREQ.";
}
