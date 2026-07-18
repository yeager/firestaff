#ifndef FIRESTAFF_REDMCSB_F1007_ADD_MEMORY_CHUNK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1007_ADD_MEMORY_CHUNK_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB MEMORY.C:177-188, MEDIA758_A36M_A35E_A35M.
 *
 * The chunk header occupies the bottom of caller-owned storage. F1007
 * prepends that header to the original global chunk list and makes the
 * remaining storage available for top-down allocations.
 */
typedef struct redmcsb_f1007_memory_chunk_pc34_compat {
    struct redmcsb_f1007_memory_chunk_pc34_compat *next;
    int32_t total_byte_count;
    unsigned char *top;
    int32_t available_byte_count;
    int16_t requirements;
    int16_t in_use;
    unsigned char *backup_top;
    int32_t backup_available_byte_count;
} redmcsb_f1007_memory_chunk_pc34_compat;

extern redmcsb_f1007_memory_chunk_pc34_compat
    *redmcsb_f1007_first_memory_chunk_pc34_compat;

void F1007_AddMemoryChunk_PC34_Compat(
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk,
    int32_t byte_count,
    int16_t requirements);

/* ReDMCSB source-named alias for F1007_AddMemoryChunk_PC34_Compat. */
void F1007_AddMemoryChunk(
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk,
    int32_t byte_count,
    int16_t requirements);

const char *redmcsb_f1007_add_memory_chunk_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1007_ADD_MEMORY_CHUNK_PC34_COMPAT_H */
