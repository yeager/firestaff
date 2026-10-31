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
    return "ReDMCSB PRIM2C.C:106-123 defines "
           "F0919_PRIM_19_File_Seek: the PC 3.4 SEEK(handle, offset, 0) "
           "route returns 1 unless the resulting position equals offset; "
           "PRIM.H:323 maps F6017 to the PrimSeek slot 19.";
}
