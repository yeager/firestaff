#include "dm1_v1_f1506_f1525_source_ownership_pc34_compat.h"

static const DM1_V1_F1506F1525OwnershipPc34 kOwnership[] = {
    {1506, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1506_SetMouseInputFromSwitchOptions", "SWITCH.C:230", "No authenticated PC34 switch-option/input receipt; no generated input route."},
    {1507, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1507_DrawSwitchOptionBitmap", "SWITCH.C:256", "No authenticated PC34 switch-option bitmap/palette receipt; no generated graphics."},
    {1508, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1508_FreeMemoryOfAllSwitchOptionGraphics", "SWITCH.C:298", "No authenticated PC34 switch-option graphics ownership route."},
    {1509, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1509_Initialization", "SWITCH.C:310", "No authenticated PC34 switch-option initialization route."},
    {1510, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1510_LoadDataFile", "SWITCH.C:329", "No authenticated PC34 switch data-file receipt; no generated data path."},
    {1511, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1511_Unreferenced", "SWITCH.C:404", "Unreferenced switch route; no authenticated PC34 owner."},
    {1512, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1512", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1513, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1513_MainLoop", "SWITCH.C:412", "No authenticated PC34 switch main-loop route; no synthetic startup/UI loop."},
    {1514, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1514", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1515, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1515", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1516, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1516", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1517, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1517", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1518, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1518", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1519, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1519", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1520, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1520", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1521, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1521", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1522, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1522", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1523, DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34, "slot 1523", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1524, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1524_VerticalBlankClearPalette", "UTSTWKS.C:18", "Platform VBlank palette clear lacks a PC34 route; no synthetic timing or palette write."},
    {1525, DM1_V1_F1506_F1525_PLATFORM_BOUNDARY_PC34, "F1525_OpenVDIWorkstation", "ANIM.C:98; UTSTWKS.C", "Atari VDI workstation route lacks an authenticated PC34 host receipt."},
};

const DM1_V1_F1506F1525OwnershipPc34 *
dm1_v1_f1506_f1525_source_ownership_pc34(unsigned int number)
{
    if (number < 1506U || number > 1525U) return 0;
    return &kOwnership[number - 1506U];
}

int dm1_v1_f1506_f1525_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1506_f1525_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1506_f1525_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB SWITCH.C:230-412; UTSTWKS.C:18; ANIM.C:98. Existing "
           "game-state comments are not a source-bound PC34 implementation. "
           "No generated UI, graphics, timing, palette, input, or data route.";
}
