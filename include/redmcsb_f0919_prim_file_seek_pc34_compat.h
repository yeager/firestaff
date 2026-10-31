/*
 * ReDMCSB PRIM2C.C F0919_PRIM_19_File_Seek, PC 3.4 (I34E/I34M) route.
 *
 * Slot 19 seeks from OFFSET_BEGINNING on Atari ST/FM Towns and reports
 * success as zero. This adapter models the PC 3.4 DOS backend route, where
 * the primitive returns zero only when the resulting position equals the
 * requested signed 32-bit offset.
 */
#ifndef FIRESTAFF_REDMCSB_F0919_PRIM_FILE_SEEK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0919_PRIM_FILE_SEEK_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*redmcsb_f0919_prim_file_seek_backend_pc34_compat)(
    void *context,
    int32_t file_handle,
    int32_t offset,
    int origin);

/* Returns 0 if backend's resulting position equals offset, otherwise 1. */
int redmcsb_f0919_prim_file_seek_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_prim_file_seek_backend_pc34_compat backend,
    void *context);

const char *redmcsb_f0919_prim_file_seek_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
