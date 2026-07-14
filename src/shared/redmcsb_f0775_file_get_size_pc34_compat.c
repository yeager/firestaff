#include "redmcsb_f0775_file_get_size_pc34_compat.h"

uint32_t redmcsb_f0775_file_get_size_pc34_compat(
    redmcsb_f0775_file_tell_pc34_compat file_tell,
    redmcsb_f0775_dos_seek_to_end_pc34_compat seek_to_end,
    redmcsb_f0775_file_seek_from_beginning_pc34_compat seek_from_beginning,
    void *context,
    int16_t file_handle)
{
    int32_t offset = file_tell(context, file_handle);
    uint32_t size = seek_to_end(context, file_handle);

    /* ReDMCSB returns the DOS result before F0774 restores the original mark. */
    seek_from_beginning(context, file_handle, offset);
    return size;
}

const char *redmcsb_f0775_file_get_size_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:647-680 (PC 3.4 MEDIA480_P20JB_I34E_I34M_"
           "P31J): F0775 saves F0779_FILE_Tell, performs DOS INT 21h/AH=42h "
           "with AL=2 and offset zero, restores the mark through F0774, and "
           "returns the raw 32-bit DX:AX EOF position.";
}
