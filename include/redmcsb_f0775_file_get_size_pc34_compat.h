/*
 * ReDMCSB FILE.C F0775_FILE_GetSize, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0775_FILE_GET_SIZE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0775_FILE_GET_SIZE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Host bridges for the DOS FILE.C operations.  No host file I/O is performed. */
typedef int32_t (*redmcsb_f0775_file_tell_pc34_compat)(
    void *context,
    int16_t file_handle);

typedef uint32_t (*redmcsb_f0775_dos_seek_to_end_pc34_compat)(
    void *context,
    int16_t file_handle);

typedef void (*redmcsb_f0775_file_seek_from_beginning_pc34_compat)(
    void *context,
    int16_t file_handle,
    int32_t offset);

/*
 * Returns the raw 32-bit DX:AX result of DOS seek-to-end and restores the
 * previous mark through F0774's seek-from-beginning route.
 */
uint32_t redmcsb_f0775_file_get_size_pc34_compat(
    redmcsb_f0775_file_tell_pc34_compat file_tell,
    redmcsb_f0775_dos_seek_to_end_pc34_compat seek_to_end,
    redmcsb_f0775_file_seek_from_beginning_pc34_compat seek_from_beginning,
    void *context,
    int16_t file_handle);

const char *redmcsb_f0775_file_get_size_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
