#ifndef FIRESTAFF_REDMCSB_F1017_MALLOC_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1017_MALLOC_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CEDT018.C F1017_Malloc belongs to a non-PC media route.  The PC 3.4
 * compatibility surface is intentionally a boundary: it does not replace the
 * source allocator with host malloc.
 */
void *redmcsb_f1017_malloc_pc34_compat(size_t byte_count);

const char *redmcsb_f1017_malloc_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1017_MALLOC_PC34_COMPAT_H */
