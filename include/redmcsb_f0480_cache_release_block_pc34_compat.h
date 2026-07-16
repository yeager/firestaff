#ifndef FIRESTAFF_REDMCSB_F0480_CACHE_RELEASE_BLOCK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0480_CACHE_RELEASE_BLOCK_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ReDMCSB MEMORY.C F0480_CACHE_ReleaseBlock (lines 1487-1586).
 *
 * The original selects a derived-bitmap slot with bit 15, removes the
 * corresponding cached block from its used list, clears that lookup, and
 * returns its range to the free cache area. This adapter models only those
 * mutations in caller-owned, bounded tables; it does not own cache memory.
 */
#define REDMCSB_F0480_CACHE_BLOCK_NONE UINT16_MAX
#define REDMCSB_F0480_CACHE_DERIVED_BITMAP_MASK UINT16_C(0x8000)

typedef struct {
    size_t offset;
    size_t size;
} ReDMCSBF0480CacheFreeSpanPc34Compat;

typedef struct {
    size_t offset;
    size_t size;
    uint16_t bitmap_index;
    uint16_t previous_used_index;
    uint16_t next_used_index;
    bool allocated;
} ReDMCSBF0480CacheBlockPc34Compat;

typedef struct {
    ReDMCSBF0480CacheBlockPc34Compat *blocks;
    size_t block_count;
    uint16_t *native_block_indices;
    size_t native_block_index_count;
    uint16_t *derived_block_indices;
    size_t derived_block_index_count;
    ReDMCSBF0480CacheFreeSpanPc34Compat *free_spans;
    size_t free_span_count;
    size_t free_span_capacity;
    size_t cache_size;
    uint16_t first_used_index;
    uint16_t last_used_index;
} ReDMCSBF0480CacheReleaseStatePc34Compat;

/* Releases bitmap_index, whose high bit selects the derived lookup table.
 * Returns false and preserves state when a table, list, or span invariant is
 * invalid, the target is absent, or the bounded free-span table has no room.
 */
bool F0480_CACHE_ReleaseBlock_PC34(
    uint16_t bitmap_index,
    ReDMCSBF0480CacheReleaseStatePc34Compat *state);

#endif
