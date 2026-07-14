#ifndef FIRESTAFF_REDMCSB_F0929_PRIM_FTL_LOAD_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0929_PRIM_FTL_LOAD_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PRIM2B.C F0929_PRIM_05_FTL_Load, PC 3.4 route. */
#define REDMCSB_F0929_FTL_HEADER_SIZE_PC34 20u
#define REDMCSB_F0929_FTL_SEGMENT_HEADER_SIZE_PC34 12u
#define REDMCSB_F0929_FTL_JUMP_HEADER_SIZE_PC34 40u
#define REDMCSB_F0929_FTL_DATA_HEADER_SIZE_PC34 10u

typedef int (*redmcsb_f0929_read_pc34_compat)(
    void *context, int32_t handle, void *buffer, uint32_t byte_count);
typedef int (*redmcsb_f0929_seek_pc34_compat)(
    void *context, int32_t handle, uint32_t offset);
typedef void *(*redmcsb_f0929_allocate_pc34_compat)(
    void *context, uint32_t byte_count);
typedef void (*redmcsb_f0929_free_pc34_compat)(void *context, void *memory);
typedef int (*redmcsb_f0929_decompress_code_pc34_compat)(
    void *context, const uint8_t *source, uint8_t *destination);

typedef struct {
    void *context;
    redmcsb_f0929_read_pc34_compat read;
    redmcsb_f0929_seek_pc34_compat seek;
    redmcsb_f0929_allocate_pc34_compat allocate;
    redmcsb_f0929_free_pc34_compat free;
    redmcsb_f0929_decompress_code_pc34_compat decompress_code;
} redmcsb_f0929_callbacks_pc34_compat;

typedef struct {
    uint8_t *memory_address;
    uint8_t *a5_world;
    uint8_t *jump_table_address;
    uint32_t stack_size;
} redmcsb_f0929_ftl_executable_pc34_compat;

/* Returns the original F0929 loader status (zero on success). */
uint16_t redmcsb_f0929_prim_ftl_load_pc34_compat(
    int32_t file_handle,
    redmcsb_f0929_ftl_executable_pc34_compat *executable,
    const redmcsb_f0929_callbacks_pc34_compat *callbacks);

const char *redmcsb_f0929_prim_ftl_load_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0929_PRIM_FTL_LOAD_PC34_COMPAT_H */
