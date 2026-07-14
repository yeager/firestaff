#include "redmcsb_f0926_floppy_force_media_change_detection_pc34_compat.h"

bool redmcsb_f0926_floppy_force_media_change_detection_pc34_compat(
    int device_number)
{
    (void)device_number;
    return false;
}

const char *
redmcsb_f0926_floppy_force_media_change_detection_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C:297-395 defines F0926 only under MEDIA772_SU1E: "
           "it changes G0720_ac_ from A:\\F using the device number, enters "
           "supervisor mode with Super(0L), temporarily installs 68000 "
           "handlers at 0x0472, 0x047E, and 0x0476, calls Fopen, then restores "
           "the saved vectors when still installed. No PC 3.4 branch or "
           "portable host adapter is supplied.";
}
