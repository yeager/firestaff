#include "redmcsb_f1007_add_memory_chunk_pc34_compat.h"

#include <stddef.h>

redmcsb_f1007_memory_chunk_pc34_compat
    *redmcsb_f1007_first_memory_chunk_pc34_compat;

void F1007_AddMemoryChunk_PC34_Compat(
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk,
    int32_t byte_count,
    int16_t requirements)
{
    memory_chunk->next = redmcsb_f1007_first_memory_chunk_pc34_compat;
    memory_chunk->total_byte_count = byte_count - (int32_t)sizeof(*memory_chunk);
    memory_chunk->requirements = requirements;
    memory_chunk->in_use = 0;
    memory_chunk->backup_top = memory_chunk->top =
        (unsigned char *)memory_chunk + memory_chunk->total_byte_count +
        sizeof(*memory_chunk);
    memory_chunk->backup_available_byte_count =
        memory_chunk->available_byte_count = memory_chunk->total_byte_count;
    redmcsb_f1007_first_memory_chunk_pc34_compat = memory_chunk;
}

const char *redmcsb_f1007_add_memory_chunk_source_evidence(void)
{
    return "ReDMCSB MEMORY.C:177-188, MEDIA758_A36M_A35E_A35M: "
           "F1007 prepends MEMORY_CHUNK to G2137_ps_FirstMemoryChunk, sets "
           "TotalByteCount to byte count minus sizeof(MEMORY_CHUNK), clears "
           "InUse, and initializes current and backup top/count fields.";
}
