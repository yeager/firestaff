#include "redmcsb_f0778_file_rename_pc34_compat.h"

void redmcsb_f0778_file_rename_pc34_compat(
    redmcsb_f0778_dos_rename_pc34_compat rename_file,
    void *context,
    const char *source_file_name,
    const char *destination_file_name)
{
    /* ReDMCSB FILE.C:714-725, PC 3.4 route. */
    rename_file(context, source_file_name, destination_file_name);
}

const char *redmcsb_f0778_file_rename_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:714-725 (PC 3.4): F0778_FILE_Rename issues "
           "DOS INT 21h/AH=56h with DS:DX pointing to the ASCIZ old path "
           "and ES:DI pointing to the ASCIZ new path, then returns void "
           "without checking the DOS result.";
}
