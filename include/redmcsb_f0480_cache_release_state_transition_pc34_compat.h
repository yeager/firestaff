#ifndef FIRESTAFF_REDMCSB_F0480_CACHE_RELEASE_STATE_TRANSITION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0480_CACHE_RELEASE_STATE_TRANSITION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define REDMCSB_F0480_RELEASE_BITMAP_DERIVED_MASK UINT16_C(0x8000)
#define REDMCSB_F0480_RELEASE_BITMAP_NONE UINT16_MAX

/* This is the source block's two overlaid roles: a used bitmap block and an
 * unused cache block. The unused links are meaningful only after release. */
typedef struct ReDMCSBF0480CacheReleaseBlockPc34Compat {
    size_t address;
    size_t byte_count;
    uint16_t bitmap_index;
    struct ReDMCSBF0480CacheReleaseBlockPc34Compat *previous_used_block;
    struct ReDMCSBF0480CacheReleaseBlockPc34Compat *next_used_block;
    struct ReDMCSBF0480CacheReleaseBlockPc34Compat *previous_unused_block;
    struct ReDMCSBF0480CacheReleaseBlockPc34Compat *next_unused_block;
} ReDMCSBF0480CacheReleaseBlockPc34Compat;

typedef struct {
    ReDMCSBF0480CacheReleaseBlockPc34Compat **native_blocks;
    size_t native_block_count;
    ReDMCSBF0480CacheReleaseBlockPc34Compat **derived_blocks;
    size_t derived_block_count;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *first_used_block;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *last_used_block;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *first_referenced_used_block;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *first_unused_block;
    size_t available_byte_count;
    size_t cache_top;
} ReDMCSBF0480CacheReleaseStateTransitionPc34Compat;

/* Implements ReDMCSB MEMORY.C:1487-1586. The derived bitmap bit selects the
 * derived lookup table. On success this clears that table entry, unlinks the
 * used block, coalesces address-adjacent unused blocks, and contracts an
 * unused range ending at cache_top. */
bool F0480_CACHE_ReleaseBlock_StateTransition_PC34(
    uint16_t bitmap_index,
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat *state);

#endif
