#include "dm1_v1_f1446_f1465_local_ownership_pc34_compat.h"

static const DM1_V1_F1446F1465LocalOwnershipPc34 kOwnership[] = {
    {1446, "L1446_l_ReplacementBPBVector", "F0450_FLOPPY_ForceMediaChangeDetection", "FLOPPYST.C"},
    {1447, "L1447_i_FloppyDriveIndex", "F0450_FLOPPY_ForceMediaChangeDetection", "FLOPPYST.C"},
    {1448, "L1448_pc_DefaultBPBVector", "F0450_FLOPPY_ForceMediaChangeDetection", "FLOPPYST.C"},
    {1449, "L1449_pc_DefaultMediaChangeVector", "F0450_FLOPPY_ForceMediaChangeDetection", "FLOPPYST.C"},
    {1450, "L1450_pc_DefaultRWBlocksVector", "F0450_FLOPPY_ForceMediaChangeDetection", "FLOPPYST.C"},
    {1451, "L1451_i_FloppyDriveCount", "F0451_FLOPPY_Initialize", "FLOPPY.C"},
    {1452, "L1452_l_SupervisorStack", "F0451_FLOPPY_Initialize", "FLOPPY.C"},
    {1453, "L1453_ui_DiskType", "F0452_FLOPPY_GetDiskTypeInDrive_CPSB", "FLOPPY.C"},
    {1454, "L1454_i_FloppyDriveIndex", "F0452_FLOPPY_GetDiskTypeInDrive_CPSB", "FLOPPY.C"},
    {1455, "L1455_i_GraphicsDatFileReferenceCountBackup", "F0452_FLOPPY_GetDiskTypeInDrive_CPSB", "FLOPPY.C"},
    {1456, "L1456_puc_Buffer_CPSB", "F0452_FLOPPY_GetDiskTypeInDrive_CPSB", "FLOPPY.C"},
    {1457, "L1457_ui_TrackIndex", "F0453_FLOPPY_IsFormatDiskSuccessful", "FLOPPY.C; UTIO.C"},
    {1458, "L1458_B_IsFormatDiskSuccessful", "F0453_FLOPPY_IsFormatDiskSuccessful", "FLOPPY.C; UTIO.C"},
    {1459, "L1459_l_ByteCount", "F0453_FLOPPY_IsFormatDiskSuccessful", "FLOPPY.C; UTIO.C"},
    {1460, "L1460_puc_Buffer", "F0453_FLOPPY_IsFormatDiskSuccessful", "FLOPPY.C; UTIO.C"},
    {1461, "L1461_ui_Multiple", "F0454_FLOPPY_IsSaveDiskTypeInSaveDiskDrive", "FLOPPY.C"},
    {1462, "L1462_i_Multiple", "F0457_START_DrawEnabledMenus_CPSF", "STARTUP2.C"},
    {1463, "L1463_i_CommandLineLength", "F0458_START_GetCommandLineParameters_CPSA", "STARTUP2.C"},
    {1464, "L1464_i_Character", "F0458_START_GetCommandLineParameters_CPSA", "STARTUP2.C"},
    {1465, "L1465_puc_CommandLineParameters", "F0458_START_GetCommandLineParameters_CPSA", "STARTUP2.C"},
};

const DM1_V1_F1446F1465LocalOwnershipPc34 *
dm1_v1_f1446_f1465_local_ownership_pc34(unsigned int number)
{
    if (number < 1446U || number > 1465U) return 0;
    return &kOwnership[number - 1446U];
}

int dm1_v1_f1446_f1465_admits_standalone_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1446_f1465_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1446_f1465_local_ownership_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYST.C F0450; FLOPPY.C F0451-F0454; STARTUP2.C "
           "F0457-F0458. All F1446-F1465 identifiers are L-local storage; "
           "no generated disk, format, startup, graphics, timing, or UI route exists.";
}
