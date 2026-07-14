#include "redmcsb_f0942_init_disk_resource_pc34_compat.h"

bool redmcsb_f0942_init_disk_resource_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0942_init_disk_resource_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0942_InitDiskResource with "
           "MEDIA442_A20E_A21E; EXEC.C:379-395 calls "
           "OpenResource(\"disk.resource\"), AllocMem for "
           "DiscResourceUnit, CreatePort, and assigns mn_ReplyPort. No PC "
           "3.4 branch or portable host adapter is supplied.";
}
