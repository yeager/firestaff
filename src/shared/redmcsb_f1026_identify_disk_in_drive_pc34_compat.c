#include "redmcsb_f1026_identify_disk_in_drive_pc34_compat.h"

bool redmcsb_f1026_identify_disk_in_drive_pc34_compat(int16_t drive_pda)
{
    (void)drive_pda;
    return false;
}

const char *
redmcsb_f1026_identify_disk_in_drive_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPY.C:458-543 encloses "
           "F1026_IdentifyDiskInDrive_CPSX in MEDIA607_X30J_X31J. "
           "FLOPPY.C:470-520 reads X68000 IOCS B_READ sector 9 twice "
           "through TRAP 15, compares its HPR-0007 identifier, and returns "
           "game disk when the weak-byte sums differ. FLOPPY.C:522-540 "
           "clears G3091_i_ErrorCount, reads sector 6 through the same IOCS "
           "service, and returns save disk only when F1031 reports success; "
           "otherwise it returns unformatted disk. FLOPPY.C:970-988 consumes "
           "the source values 0 (unformatted), 1 (game), and 2 (save). No PC "
           "3.4 branch or portable host adapter is supplied.";
}
