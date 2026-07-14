#include "redmcsb_f0779_file_tell_pc34_compat.h"

int32_t redmcsb_f0779_file_tell_pc34_compat(
    redmcsb_f0779_dos_tell_pc34_compat tell,
    void *context,
    int16_t file_handle)
{
    return tell(context, file_handle);
}

const char *redmcsb_f0779_file_tell_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:624-646 (PC 3.4 MEDIA459_P20JA_P20JB_P31J, "
           "MEDIA472_P20JB_P31J): F0779_FILE_Tell invokes DOS INT "
           "21h/AH=42h with AL=1 and zero CX:DX to obtain the current "
           "file mark. The portable bridge preserves that returned signed "
           "32-bit mark without performing host file I/O.";
}
