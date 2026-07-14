#include "redmcsb_f1017_malloc_pc34_compat.h"

void *redmcsb_f1017_malloc_pc34_compat(size_t byte_count)
{
    (void)byte_count;
    return NULL;
}

const char *redmcsb_f1017_malloc_source_evidence_pc34(void)
{
    return "ReDMCSB CEDT018.C F1017_Malloc is selected only by a non-PC "
           "media route. No PC 3.4 branch or portable host allocator adapter "
           "is present; this compatibility boundary therefore returns NULL "
           "without allocating.";
}
