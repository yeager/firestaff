#include "redmcsb_f1007_add_memory_chunk_pc34_compat.h"
#include "redmcsb_f1008_get_largest_available_memory_chunk.h"
#include "redmcsb_f1017_malloc_pc34_compat.h"
#include "redmcsb_f1018_mfree_pc34_compat.h"

#include <stddef.h>

redmcsb_f1007_memory_chunk_pc34_compat
    *redmcsb_f1007_first_memory_chunk_pc34_compat = 0;

void F1007_AddMemoryChunk_PC34_Compat(
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk,
    int32_t byte_count,
    int16_t requirements)
{
    if (memory_chunk == 0 ||
        byte_count <= (int32_t)sizeof(*memory_chunk)) {
        return;
    }

    memory_chunk->next = redmcsb_f1007_first_memory_chunk_pc34_compat;
    memory_chunk->total_byte_count = byte_count;
    memory_chunk->top = (unsigned char *)memory_chunk + (size_t)byte_count;
    memory_chunk->available_byte_count =
        byte_count - (int32_t)sizeof(*memory_chunk);
    memory_chunk->requirements = requirements;
    memory_chunk->in_use = 0;
    memory_chunk->backup_top = memory_chunk->top;
    memory_chunk->backup_available_byte_count =
        memory_chunk->available_byte_count;
    redmcsb_f1007_first_memory_chunk_pc34_compat = memory_chunk;
}

void F1007_AddMemoryChunk(
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk,
    int32_t byte_count,
    int16_t requirements)
{
    F1007_AddMemoryChunk_PC34_Compat(memory_chunk, byte_count, requirements);
}

const char *redmcsb_f1007_add_memory_chunk_source_evidence(void)
{
    return "ReDMCSB MEMORY.C:177-188 F1007_AddMemoryChunk";
}

redmcsb_f1008_memory_chunk *
F1008_GetLargestAvailableMemoryChunk(
    redmcsb_f1008_memory_chunk *first_memory_chunk,
    int16_t requirements)
{
    return redmcsb_f1008_get_largest_available_memory_chunk(
        first_memory_chunk,
        requirements);
}

void *redmcsb_f1017_malloc_pc34_compat(size_t byte_count)
{
    (void)byte_count;
    return 0;
}

void *F1017_Malloc(size_t byte_count)
{
    return redmcsb_f1017_malloc_pc34_compat(byte_count);
}

const char *redmcsb_f1017_malloc_source_evidence_pc34(void)
{
    return "ReDMCSB CEDT018.C:208 F1017_Malloc; no PC34 host malloc route";
}

bool redmcsb_f1018_mfree_pc34_compat(void)
{
    return false;
}

bool F1018_Mfree(void)
{
    return redmcsb_f1018_mfree_pc34_compat();
}

const char *redmcsb_f1018_mfree_source_evidence_pc34(void)
{
    return "ReDMCSB CEDT018.C:216 F1018_Mfree; no PC34 host Mfree route";
}
