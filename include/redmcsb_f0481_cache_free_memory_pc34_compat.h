#ifndef FIRESTAFF_REDMCSB_F0481_CACHE_FREE_MEMORY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0481_CACHE_FREE_MEMORY_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ReDMCSB MEMORY.C F0481_CACHE_FreeMemory (lines 462-471).
 *
 * The original repeatedly releases the bitmap at the first used cache block
 * until enough cache bytes are available. This adapter leaves block ownership
 * to the caller; the release callback must publish the next first-used bitmap
 * and the new available byte count before it returns.
 */
#define REDMCSB_F0481_CACHE_BITMAP_NONE UINT16_MAX

typedef bool (*ReDMCSBF0481CacheReleaseBlockPc34Compat)(
    uint16_t bitmap_index,
    void *context);

typedef struct {
    size_t available_cache_memory_byte_count;
    size_t cache_memory_capacity;
    uint16_t first_used_bitmap_index;
    ReDMCSBF0481CacheReleaseBlockPc34Compat release_block;
    void *release_context;
} ReDMCSBF0481CacheFreeMemoryStatePc34Compat;

/* Ensures byte_count is available by repeatedly releasing the current first
 * used bitmap. Returns false for invalid bounded state, an empty used list,
 * or a failed release callback.
 */
bool F0481_CACHE_FreeMemory_PC34(
    size_t byte_count,
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat *state);

#endif
