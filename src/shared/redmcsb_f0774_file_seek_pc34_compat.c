#include "redmcsb_f0774_file_seek_pc34_compat.h"

bool redmcsb_f0774_file_seek_pc34_compat(
    redmcsb_f0774_dos_seek_from_beginning_pc34_compat seek_from_beginning,
    void *context,
    int16_t file_handle,
    int32_t offset)
{
    /* ReDMCSB WIP20210206 FILE.C:606-621, PC 3.4 route. */
    return seek_from_beginning(context, file_handle, offset);
}

const char *redmcsb_f0774_file_seek_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:606-621 (PC 3.4 MEDIA463_P20JA_P20JB_I34E_"
           "I34M_P31J): F0774_FILE_Seek issues DOS INT 21h/AH=42h with "
           "AL=0, passes the signed 32-bit offset from the file beginning, "
           "and returns true only when carry is clear.";
}
