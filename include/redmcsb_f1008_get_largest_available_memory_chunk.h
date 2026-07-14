#ifndef FIRESTAFF_REDMCSB_F1008_GET_LARGEST_AVAILABLE_MEMORY_CHUNK_H
#define FIRESTAFF_REDMCSB_F1008_GET_LARGEST_AVAILABLE_MEMORY_CHUNK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB MEMORY.C F1008 reads these fields from MEMORY_CHUNK. The omitted
 * allocation-cursor fields are not observed by this selector.
 */
typedef struct redmcsb_f1008_memory_chunk {
    struct redmcsb_f1008_memory_chunk *next;
    int32_t total_byte_count;
    int16_t requirements;
    int16_t in_use;
} redmcsb_f1008_memory_chunk;

/*
 * Returns the unused eligible chunk with the largest total byte count and
 * marks it in use. Equal-sized chunks retain first-list precedence.
 */
redmcsb_f1008_memory_chunk *
redmcsb_f1008_get_largest_available_memory_chunk(
    redmcsb_f1008_memory_chunk *first_memory_chunk,
    int16_t requirements);

const char *redmcsb_f1008_get_largest_available_memory_chunk_source_evidence(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1008_GET_LARGEST_AVAILABLE_MEMORY_CHUNK_H */
