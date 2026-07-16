#include "redmcsb_f1008_get_largest_available_memory_chunk.h"

redmcsb_f1008_memory_chunk *
redmcsb_f1008_get_largest_available_memory_chunk(
    redmcsb_f1008_memory_chunk *first_memory_chunk,
    int16_t requirements)
{
    int32_t largest_memory_chunk_size = 0;
    redmcsb_f1008_memory_chunk *memory_chunk = first_memory_chunk;
    redmcsb_f1008_memory_chunk *selected_memory_chunk = 0;

    while (memory_chunk != 0) {
        if (memory_chunk->in_use == 0 &&
            (memory_chunk->requirements & requirements) == requirements &&
            largest_memory_chunk_size < memory_chunk->total_byte_count) {
            selected_memory_chunk = memory_chunk;
            largest_memory_chunk_size = memory_chunk->total_byte_count;
        }
        memory_chunk = memory_chunk->next;
    }

    if (selected_memory_chunk != 0) {
        selected_memory_chunk->in_use = 1;
    }
    return selected_memory_chunk;
}

const char *redmcsb_f1008_get_largest_available_memory_chunk_source_evidence(
    void)
{
    return "ReDMCSB MEMORY.C:191-220 F1008_GetLargestAvailableMemoryChunk "
           "scans G2137_ps_FirstMemoryChunk, accepts only C0_FALSE chunks "
           "whose Requirements contain the requested mask, retains a chunk "
           "only when TotalByteCount is strictly larger, and sets the "
           "selected chunk InUse to C1_TRUE.";
}
