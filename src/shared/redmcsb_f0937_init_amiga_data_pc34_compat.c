#include "redmcsb_f0937_init_amiga_data_pc34_compat.h"

bool redmcsb_f0937_init_amiga_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0937_init_amiga_data_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0937_InitAmigaData with "
           "MEDIA442_A20E_A21E; EXEC.C:175-273 uses FindTask, AvailMem, "
           "graphics.library, Chip RAM, copper lists, and Amiga view/bitmap "
           "setup. No PC 3.4 branch or portable host adapter is supplied.";
}
