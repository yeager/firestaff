#include "redmcsb_f0770_file_open_pc34_compat.h"

int16_t redmcsb_f0770_file_open_pc34_compat(
    redmcsb_f0770_dos_open_pc34_compat open,
    void *context,
    const char *file_name)
{
    int16_t file_handle;

    if (open(context, file_name, REDMCSB_F0770_DOS_OPEN_READ_WRITE_PC34,
             &file_handle)) {
        return file_handle;
    }
    return INT16_C(-1);
}

const char *redmcsb_f0770_file_open_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:470-489 (PC 3.4 MEDIA463_P20JA_P20JB_I34E_"
           "I34M_P31J): F0770_FILE_Open issues DOS INT 21h/AH=3Dh with "
           "AL=2 (read/write) and returns AX, or 0xFFFF when carry is set.";
}
