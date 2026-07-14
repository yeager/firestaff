#include "redmcsb_f0939_release_amiga_data_pc34_compat.h"

bool redmcsb_f0939_release_amiga_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0939_release_amiga_data_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0939_ReleaseAmigaData with "
           "MEDIA442_A20E_A21E; EXEC.C:294-330 releases audio/disk data, "
           "Amiga views, copper lists, bitmap memory, and graphics.library, "
           "then calls exit(0). No PC 3.4 branch or portable host adapter is "
           "supplied.";
}
