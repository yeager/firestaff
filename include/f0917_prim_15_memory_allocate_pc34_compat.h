#ifndef FIRESTAFF_F0917_PRIM_15_MEMORY_ALLOCATE_PC34_COMPAT_H
#define FIRESTAFF_F0917_PRIM_15_MEMORY_ALLOCATE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB PRIM1.C:10 F0917_PRIM_15_Memory_Allocate takes a PC 3.4
 * signed-long byte count and returns the platform allocator's result.
 * The host supplies that allocator; this adapter never creates a heap.
 */
typedef void *(*f0917_prim_15_memory_allocate_callback_pc34_compat)(
    void *context,
    size_t byte_count);

typedef struct {
    void *context;
    f0917_prim_15_memory_allocate_callback_pc34_compat allocate;
} f0917_prim_15_memory_allocator_pc34_compat;

/*
 * For a non-negative PC 3.4 byte count, calls allocate exactly once and
 * returns its result unchanged. Negative counts and an absent callback return
 * NULL without a callback. Zero is forwarded because PRIM1 delegates that
 * case to the platform allocator.
 */
void *f0917_prim_15_memory_allocate_pc34_compat(
    const f0917_prim_15_memory_allocator_pc34_compat *allocator,
    int32_t byte_count);

const char *f0917_prim_15_memory_allocate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
