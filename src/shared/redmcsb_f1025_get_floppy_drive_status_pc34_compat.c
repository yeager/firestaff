#include "redmcsb_f1025_get_floppy_drive_status_pc34_compat.h"

bool redmcsb_f1025_get_floppy_drive_status_pc34_compat(int16_t drive_pda)
{
    (void)drive_pda;
    return false;
}

const char *
redmcsb_f1025_get_floppy_drive_status_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:754 encloses F1025_GetFloppyDriveStatus in "
           "MEDIA607_X30J_X31J; FILE.C:1128-1151 invokes X68000 IOCS "
           "B_DRVCHK (D0=0x4E) through TRAP 15 with PDA 0x90 or 0x91, "
           "then clears G3091_i_ErrorCount. It returns 2 when bit 2 says "
           "not ready, 1 when bit 3 says protected, otherwise 0. FLOPPY.C:"
           "970-976 maps status 2 to no disk and status 1 to write-protected. "
           "No PC 3.4 branch or portable host adapter is supplied.";
}
