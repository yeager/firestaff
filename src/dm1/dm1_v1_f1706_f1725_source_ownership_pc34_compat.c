#include "dm1_v1_f1706_f1725_source_ownership_pc34_compat.h"

static const DM1_V1_F1706F1725OwnershipPc34 kOwnership[] = {
    {1706, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1706_ppuc_TrackContent", "FLOPPYAM.C F0519", "Function-local Amiga floppy storage; no standalone F1706 route."},
    {1707, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1707_pl_TrackContent", "FLOPPYAM.C F0519", "Function-local Amiga floppy storage; no standalone F1707 route."},
    {1708, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "slot 1708", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1709, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "slot 1709", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1710, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "slot 1710", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1711, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1711_", "MUSCMIDI.C", "Unnamed MUSC MIDI route has no authenticated PC34 music data/backend receipt."},
    {1712, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1712_ArgumentIndex", "FLOPPYAM.C F0522", "Function-local Amiga floppy storage; no standalone F1712 route."},
    {1713, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1713_pl_StandardPacketArguments", "FLOPPYAM.C F0522", "Function-local Amiga floppy storage; no standalone F1713 route."},
    {1714, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1714_l_DOSActionResult", "FLOPPYAM.C F0522", "Function-local Amiga floppy storage; no standalone F1714 route."},
    {1715, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1715_MUSC_07_", "MUSCMAIN.C:5; MUSCMIDI.C", "MUSC library route has no authenticated PC34 library/media receipt."},
    {1716, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1716_MUSC_05_StopMIDIMusic", "MUSCMAIN.C:7; MUSCMIDI.C", "MUSC stop route has no authenticated PC34 library/media receipt."},
    {1717, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1717_MUSC_04_PlayMIDIMusic", "MUSCMAIN.C:8; MUSCMIDI.C", "MUSC play route has no authenticated PC34 library/media receipt."},
    {1718, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1718_MUSC_03_Expunge", "MUSCMAIN.C:9; MUSCMIDI.C", "MUSC lifecycle route has no authenticated PC34 library/media receipt."},
    {1719, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1719_l_DiskChangeHandlerData", "FLOPPYAM.C F0525", "Function-local Amiga floppy storage; no standalone F1719 route."},
    {1720, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1720_i_DiskState", "FLOPPYAM.C F0525", "Function-local Amiga floppy storage; no standalone F1720 route."},
    {1721, DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34, "F1721_MUSC_06_", "MUSCMAIN.C:6; MUSCOPM.C", "MUSC/OPM library route has no authenticated PC34 library/media receipt."},
    {1722, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1722_B_FloppyWriteProtectionState", "FLOPPY.C F0529", "Function-local floppy storage; no standalone F1722 route."},
    {1723, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1723_pB_", "FLOPPY.C F0529", "Function-local floppy storage; no standalone F1723 route."},
    {1724, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1724_i_FloppyDriveIndex", "FLOPPY.C F0529", "Function-local floppy storage; no standalone F1724 route."},
    {1725, DM1_V1_F1706_F1725_LOCAL_OR_UNASSIGNED_PC34, "L1725_i_Result", "FLOPPY.C F0530", "Function-local floppy storage; no standalone F1725 route."},
};

const DM1_V1_F1706F1725OwnershipPc34 *
dm1_v1_f1706_f1725_source_ownership_pc34(unsigned int number)
{
    if (number < 1706U || number > 1725U) return 0;
    return &kOwnership[number - 1706U];
}

int dm1_v1_f1706_f1725_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1706_f1725_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1706_f1725_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYAM.C F0519/F0522/F0525; FLOPPY.C F0529-F0530; "
           "MUSCMAIN.C:5-9; MUSCMIDI.C; MUSCOPM.C. Existing F0813-F0815 "
           "MIDI driver handoffs remain separate. No generated music, MIDI, "
           "floppy, input, UI, graphics, or timing route.";
}
