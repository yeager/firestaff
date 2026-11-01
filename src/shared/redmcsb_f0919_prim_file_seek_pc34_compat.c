#include "redmcsb_f0919_prim_file_seek_pc34_compat.h"

int redmcsb_f0919_prim_file_seek_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_prim_file_seek_backend_pc34_compat backend,
    void *context)
{
    if (backend(context, file_handle, offset, 0) != offset) {
        return 1;
    }
    return 0;
}

const char *redmcsb_f0919_prim_file_seek_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM2C.C:106-123 defines the X68000 status route: "
           "SEEK(handle, offset, 0) returns 1 unless the resulting position "
           "equals offset; PRIM.H:323 maps F6017 to PrimSeek slot 19. "
           "PRIM1.C:40-53 defines FM Towns error-code-3 handling, and "
           "PRIM2C.C:106-118 defines the Amiga library-bracketed route.";
}

void redmcsb_f0919_prim_file_seek_fmtowns_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_prim_file_seek_backend_pc34_compat backend,
    redmcsb_f0919_prim_seek_error_pc34_compat print_os_error,
    void *context)
{
    if (backend(context, file_handle, offset, 0) != offset && print_os_error) {
        print_os_error(context, 3);
    }
}

void redmcsb_f0919_prim_file_seek_amiga_pc34_compat(
    int32_t file_handle,
    int32_t offset,
    redmcsb_f0919_amiga_library_boundary_pc34_compat begin_library,
    redmcsb_f0919_amiga_seek_pc34_compat seek,
    redmcsb_f0919_amiga_library_boundary_pc34_compat end_library,
    void *context)
{
    begin_library(context);
    (void)seek(context, file_handle, offset, 0);
    end_library(context);
}
