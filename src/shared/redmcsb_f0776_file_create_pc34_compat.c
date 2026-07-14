#include "redmcsb_f0776_file_create_pc34_compat.h"

int16_t redmcsb_f0776_file_create_pc34_compat(
    redmcsb_f0776_dos_create_pc34_compat create,
    void *context,
    const char *file_name)
{
    int16_t file_handle;

    if (create(context, file_name, REDMCSB_F0776_DOS_CREATE_ATTRIBUTES_PC34,
               &file_handle)) {
        return file_handle;
    }
    return INT16_C(-1);
}

const char *redmcsb_f0776_file_create_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:683-701 (PC 3.4 C03_GAME/C06_CEDT): "
           "F0776_FILE_Create issues DOS INT 21h/AH=3Ch with CX=0 and "
           "returns AX, or 0xFFFF when carry is set.";
}
