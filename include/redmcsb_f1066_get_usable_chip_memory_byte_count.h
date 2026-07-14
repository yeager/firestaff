#ifndef FIRESTAFF_REDMCSB_F1066_GET_USABLE_CHIP_MEMORY_BYTE_COUNT_H
#define FIRESTAFF_REDMCSB_F1066_GET_USABLE_CHIP_MEMORY_BYTE_COUNT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Portable boundary for the Amiga Exec calls used by AMIGINIT.C F1066.
 * The callbacks provide chip-memory availability, temporary allocation, the
 * system lock, and the source's system-error alert.
 */
typedef struct redmcsb_f1066_chip_memory_io {
    void (*forbid)(void *context);
    int32_t (*available_chip_memory_byte_count)(void *context,
                                                 int largest_block);
    void *(*allocate_chip_memory)(void *context, int32_t byte_count);
    void (*free_chip_memory)(void *context, void *memory,
                             int32_t byte_count);
    void (*alert_csb_system_error)(void *context, uint32_t error_code);
    void (*permit)(void *context);
} redmcsb_f1066_chip_memory_io;

/*
 * Returns the largest allocatable chip-memory block while retaining at least
 * 16K total chip memory and a remaining 8K contiguous chip-memory block.
 */
int32_t redmcsb_f1066_get_usable_chip_memory_byte_count(
    const redmcsb_f1066_chip_memory_io *io,
    void *context);

const char *redmcsb_f1066_get_usable_chip_memory_byte_count_source_evidence(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1066_GET_USABLE_CHIP_MEMORY_BYTE_COUNT_H */
