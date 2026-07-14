#ifndef FIRESTAFF_REDMCSB_F0918_PRIM_MEMORY_FREE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0918_PRIM_MEMORY_FREE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bounded adapter for ReDMCSB F0918_PRIM_16_Memory_Free.
 *
 * PRIM1.C guards a null buffer and otherwise dispatches exactly one platform
 * release call. The PC 3.4 PRIM contract exposes the same callable as
 * PrimFree, slot 16. Ownership remains with the release callback; this
 * adapter neither frees nor records the buffer.
 */
typedef bool (*ReDMCSBF0918PrimReleaseCallbackPc34Compat)(
    void *buffer,
    void *context);

/* Returns true for the source-faithful null no-op. A non-null buffer is
 * dispatched once to release_callback. Returns false when that dispatch is
 * unavailable or rejected.
 */
bool redmcsb_f0918_prim_memory_free_pc34_compat(
    void *buffer,
    ReDMCSBF0918PrimReleaseCallbackPc34Compat release_callback,
    void *release_context);

const char *redmcsb_f0918_prim_memory_free_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0918_PRIM_MEMORY_FREE_PC34_COMPAT_H */
