#include "redmcsb_f0948_release_disk_data_pc34_compat.h"

bool redmcsb_f0948_release_disk_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0948_release_disk_data_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0948_ReleaseDiskData with "
           "MEDIA442_A20E_A21E; EXEC.C:568-579 conditionally calls "
           "FreeMem for IOExtTD, DeletePort for IOExtTDReplyPort, and "
           "FreeMem for TrackBuffer using TD_SECTOR * NUMSECS. No PC 3.4 "
           "branch or portable host adapter is supplied.";
}
