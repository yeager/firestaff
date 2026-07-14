#include "redmcsb_f0920_prim_file_read_pc34_compat.h"

int redmcsb_f0920_prim_file_read_pc34_compat(
    int32_t file_handle,
    int32_t length,
    void *buffer,
    redmcsb_f0920_prim_file_read_backend_pc34_compat backend,
    void *context)
{
    if (backend(context, file_handle, buffer, length) != length) {
        return 1;
    }

    return 0;
}

const char *redmcsb_f0920_prim_file_read_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM2C.C:123-142 defines "
           "F0920_PRIM_20_File_Read: READ(handle, address, size) returns "
           "1 unless its count equals size, otherwise 0; PRIM.H:324 maps "
           "the PC 3.4 F6018 entry to PrimRead; PRIM2B.C:304-306 and "
           "324-326 consume nonzero as loader-read failure.";
}
