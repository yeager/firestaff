#include "redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_pc34_compat.h"

bool redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_pc34_compat(
    void)
{
    return false;
}

const char *
redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_source_evidence_pc34(
    void)
{
    return "ReDMCSB IO.C:4122-4151 defines F0811 as PC-98 DISK BIOS read-ID "
           "commands 0x1A and 0x5A, then accepts only DL=1, CL=0, DH=0, "
           "and CH=3. No portable host BIOS adapter is supplied.";
}
