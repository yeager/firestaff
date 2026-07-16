#include "redmcsb_f1020_initialize_x68000_pc34_compat.h"
#include "redmcsb_f1025_get_floppy_drive_status_pc34_compat.h"
#include "redmcsb_f1026_identify_disk_in_drive_pc34_compat.h"
#include "redmcsb_f1031_is_operation_successful.h"

bool redmcsb_f1020_initialize_x68000_pc34_compat(void)
{
    return false;
}

bool F1020_InitializeX68000(void)
{
    return redmcsb_f1020_initialize_x68000_pc34_compat();
}

const char *redmcsb_f1020_initialize_x68000_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C F1020_InitializeX68000; "
           "MEDIA607_X30J_X31J X68000-only, no PC34 route";
}

bool redmcsb_f1025_get_floppy_drive_status_pc34_compat(int16_t drive_pda)
{
    (void)drive_pda;
    return false;
}

bool F1025_GetFloppyDriveStatus(int16_t drive_pda)
{
    return redmcsb_f1025_get_floppy_drive_status_pc34_compat(drive_pda);
}

const char *
redmcsb_f1025_get_floppy_drive_status_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:1128 F1025_GetFloppyDriveStatus; "
           "X68000 IOCS B_DRVCHK only, no PC34 route";
}

bool redmcsb_f1026_identify_disk_in_drive_pc34_compat(int16_t drive_pda)
{
    (void)drive_pda;
    return false;
}

bool F1026_IdentifyDiskInDrive_CPSX(int16_t drive_pda)
{
    return redmcsb_f1026_identify_disk_in_drive_pc34_compat(drive_pda);
}

const char *
redmcsb_f1026_identify_disk_in_drive_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPY.C:461 F1026_IdentifyDiskInDrive_CPSX; "
           "X68000 IOCS weak-sector route only, no PC34 route";
}

bool redmcsb_f1031_is_operation_successful(int16_t *error_count)
{
    if (error_count == 0) {
        return false;
    }
    if (*error_count != 0) {
        *error_count = 0;
        return false;
    }
    return true;
}

bool F1031_IsOperationSuccessful(int16_t *error_count)
{
    return redmcsb_f1031_is_operation_successful(error_count);
}

const char *redmcsb_f1031_is_operation_successful_source_evidence(void)
{
    return "ReDMCSB CEDT023.C:1295 F1031_IsOperationSuccessful; "
           "nonzero TRAP14 error counter is cleared and reports failure";
}
