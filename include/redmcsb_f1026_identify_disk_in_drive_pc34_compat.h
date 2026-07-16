#ifndef FIRESTAFF_REDMCSB_F1026_IDENTIFY_DISK_IN_DRIVE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1026_IDENTIFY_DISK_IN_DRIVE_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FLOPPY.C F1026 identifies an X68000 disk through IOCS B_READ calls,
 * including an original-media weak-sector check. PC 3.4 supplies neither
 * IOCS TRAP 15 nor a source-defined host adapter for this operation.
 */
bool redmcsb_f1026_identify_disk_in_drive_pc34_compat(int16_t drive_pda);

bool F1026_IdentifyDiskInDrive_CPSX(int16_t drive_pda);

const char *
redmcsb_f1026_identify_disk_in_drive_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1026_IDENTIFY_DISK_IN_DRIVE_PC34_COMPAT_H */
