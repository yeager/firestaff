#include "redmcsb_f0947_init_disk_data_pc34_compat.h"

bool redmcsb_f0947_init_disk_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0947_init_disk_data_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0947_InitDiskData with "
           "MEDIA442_A20E_A21E; EXEC.C:547-566 allocates a TD_SECTOR * "
           "NUMSECS chip-memory TrackBuffer, allocates an IOExtTD in chip "
           "memory, calls CreatePort, reports allocation failures through "
           "Alert and F0939_ReleaseAmigaData, then assigns the port to "
           "iotd_Req.io_Message.mn_ReplyPort. No PC 3.4 branch or portable "
           "host adapter is supplied.";
}
