#include "redmcsb_f0777_file_delete_pc34_compat.h"

void redmcsb_f0777_file_delete_pc34_compat(
    redmcsb_f0777_dos_delete_pc34_compat delete_file,
    void *context,
    const char *file_name)
{
    /* ReDMCSB FILE.C:703-712, PC 3.4 C03_GAME/C06_CEDT route. */
    delete_file(context, file_name);
}

const char *redmcsb_f0777_file_delete_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:703-712 (PC 3.4 C03_GAME/C06_CEDT): "
           "F0777_FILE_Delete issues DOS INT 21h/AH=41h with DS:DX "
           "pointing to the ASCIZ path, then returns void without "
           "checking the DOS result.";
}
