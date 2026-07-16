#include "redmcsb_f0450_floppy_force_media_change_detection_pc34_compat.h"

void redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(
    uint16_t drive_type)
{
    (void)drive_type;
}

void F0450_FLOPPY_ForceMediaChangeDetection(uint16_t drive_type)
{
    redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(drive_type);
}

const char *redmcsb_f0450_floppy_force_media_change_detection_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPY.C:606-620 (PC 3.4 MEDIA278 path) returns for "
           "DRIVE_TYPE_NONE, then calls F0450 immediately before Floprd; "
           "FLOPPYST.C:41-46 and :48-126 provide F0450 bodies only under "
           "MEDIA006 or MEDIA265 Atari ST gates, neither of which includes "
           "PC 3.4 F31E. Consequently PC 3.4 proves no F0450 state mutation "
           "or callback behavior.";
}
