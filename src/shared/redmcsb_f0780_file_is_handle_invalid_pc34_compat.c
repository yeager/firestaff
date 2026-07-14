#include "redmcsb_f0780_file_is_handle_invalid_pc34_compat.h"

bool redmcsb_f0780_file_is_handle_invalid_pc34_compat(int16_t file_handle)
{
    return file_handle < 0;
}

const char *redmcsb_f0780_file_is_handle_invalid_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:747-751 (DOS-family route including PC 3.4 "
           "I34E/I34M): F0780_FILE_IsHandleInvalid returns "
           "P2179_i_FileHandle < 0.";
}
