#include "redmcsb_f0943_release_disk_resource_pc34_compat.h"

bool redmcsb_f0943_release_disk_resource_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0943_release_disk_resource_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0943_ReleaseDiskResource with "
           "MEDIA442_A20E_A21E; EXEC.C:397-406 conditionally calls "
           "DeletePort for DiscResourceUnitReplyPort, then FreeMem for "
           "DiscResourceUnit with sizeof(struct DiscResourceUnit). No PC "
           "3.4 branch or portable host adapter is supplied.";
}
