/*
 * ReDMCSB F0919_PRIM_19_File_Seek platform routes.
 *
 * The X68000 PRIM status variant returns 1 unless seek returns the requested
 * position. FM Towns uses a void route and reports a mismatch through
 * F0928_PrintOSError(3). Amiga brackets Seek with library begin/end calls
 * and ignores its result.
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

/* X68000 status variant: 0 for exact position, 1 for any other result. */
int redmcsb_f0919_prim_file_seek_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_prim_file_seek_backend_pc34_compat backend,
    void *context);

typedef void (*redmcsb_f0919_prim_seek_error_pc34_compat)(
    void *context,
    int error_code);

/* FM Towns PRIM1 route; a non-exact result dispatches OS error code 3. */
void redmcsb_f0919_prim_file_seek_fmtowns_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_prim_file_seek_backend_pc34_compat backend,
    redmcsb_f0919_prim_seek_error_pc34_compat print_os_error,
    void *context);

typedef void (*redmcsb_f0919_amiga_library_boundary_pc34_compat)(
    void *context);
typedef int32_t (*redmcsb_f0919_amiga_seek_pc34_compat)(
    void *context,
    int32_t file_handle,
    int32_t offset,
    int origin);

/* Amiga route preserves BeginLibraryFunction/Seek/EndLibraryFunction order. */
void redmcsb_f0919_prim_file_seek_amiga_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_amiga_library_boundary_pc34_compat begin_library,
    redmcsb_f0919_amiga_seek_pc34_compat seek,
    redmcsb_f0919_amiga_library_boundary_pc34_compat end_library,
    void *context);

const char *redmcsb_f0919_prim_file_seek_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
