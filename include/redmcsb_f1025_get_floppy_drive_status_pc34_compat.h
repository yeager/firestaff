#ifndef FIRESTAFF_REDMCSB_F1025_GET_FLOPPY_DRIVE_STATUS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1025_GET_FLOPPY_DRIVE_STATUS_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FILE.C F1025_GetFloppyDriveStatus is an X68000 IOCS B_DRVCHK call. PC 3.4
 * has neither the IOCS TRAP 15 service nor a source-defined host adapter.
 */
bool redmcsb_f1025_get_floppy_drive_status_pc34_compat(int16_t drive_pda);

bool F1025_GetFloppyDriveStatus(int16_t drive_pda);

const char *
redmcsb_f1025_get_floppy_drive_status_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1025_GET_FLOPPY_DRIVE_STATUS_PC34_COMPAT_H */
