#include "redmcsb_f0771_file_close_pc34_compat.h"

void redmcsb_f0771_file_close_pc34_compat(
    redmcsb_f0771_dos_close_pc34_compat close_file,
    void *context,
    int16_t file_handle)
{
    close_file(context, file_handle);
}

const char *redmcsb_f0771_file_close_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:490-497 (PC 3.4 MEDIA463_P20JA_P20JB_I34E_"
           "I34M_P31J): F0771_FILE_Close loads BX with the file handle and "
           "issues DOS INT 21h/AH=3Eh without observing its status.";
}
