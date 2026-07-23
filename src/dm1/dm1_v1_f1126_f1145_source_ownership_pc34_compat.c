#include "dm1_v1_f1126_f1145_source_ownership_pc34_compat.h"

static const DM1_V1_F1126F1145OwnershipPc34 kOwnership[] = {
    {1126, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1126_ui_Command", "CLIKCHAM.C F0367", "Function-local command storage; no standalone F1126 route."},
    {1127, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1127_TransitionFromPaletteToPalette", "PALETTE.C:136-157", "No authenticated PC34 palette-transition receipt; no generated fade is admitted."},
    {1128, DM1_V1_F1126_F1145_EXISTING_PC34_OWNER_PC34, "F1128_IsLeftMouseButtonDown", "FILLBOX.C:6", "Existing caller-owned mouse-status owner; no host polling or synthetic input."},
    {1129, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1129_", "FILLBOX.C:37-41", "Unnamed fill-box helper; no authenticated PC34 material route is evidenced."},
    {1130, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1130_ui_SymbolIndex", "CLIKMENU.C F0369", "Function-local spell-symbol storage; no standalone F1130 route."},
    {1131, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1131_InvertBox", "FILLBOX.C:738; CEDTINCN.C:42", "No independent PC34 owner is evidenced; verified F0698 remains separately owned."},
    {1132, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1132_ConvertBoxCoordinates", "BLIT.C:2036; CONVBOXC.C", "No authenticated PC34 coordinate/material receipt is evidenced."},
    {1133, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1133_AddCopperInterrupt", "AMIGINIT.C:550; COPERINT.C", "Existing Amiga Copper boundary fails closed."},
    {1134, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1134_RemoveCopperInterrupt", "AMIGINIT.C:668; COPERINT.C", "Existing Amiga Copper boundary fails closed."},
    {1135, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1135_CopperInterrupt_CPSX", "COPERINT.C:9", "Existing Amiga Copper boundary fails closed."},
    {1136, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1136_ui_MapY", "CLIKVIEW.C F0372", "Function-local sensor storage; no standalone F1136 route."},
    {1137, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1137_i_MapX", "CLIKVIEW.C F0373", "Function-local grab storage; no standalone F1137 route."},
    {1138, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1138_i_MapY", "CLIKVIEW.C F0373", "Function-local grab storage; no standalone F1138 route."},
    {1139, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1139_T_Thing", "CLIKVIEW.C F0373", "Function-local grab storage; no standalone F1139 route."},
    {1140, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1140_InitializeColorPaletteFullBlack", "AMIGAVID.C:32", "Existing Amiga palette boundary fails closed; PC34 curtain remains separately owned."},
    {1141, DM1_V1_F1126_F1145_PLATFORM_BOUNDARY_PC34, "F1141_EnablePlayfieldDisplayAndCopper", "AMIGAVID.C:50; UTAMSCR.C", "Amiga display/Copper boundary; no portable PC34 route is evidenced."},
    {1142, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1142_T_Thing", "CLIKVIEW.C F0374", "Function-local drop storage; no standalone F1142 route."},
    {1143, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1143_ps_Junk", "CLIKVIEW.C F0374", "Function-local drop storage; no standalone F1143 route."},
    {1144, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1144_i_IconIndex", "CLIKVIEW.C F0374", "Function-local drop storage; no standalone F1144 route."},
    {1145, DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34, "L1145_ui_Cell", "CLIKVIEW.C F0374", "Function-local drop storage; no standalone F1145 route."},
};

const DM1_V1_F1126F1145OwnershipPc34 *
dm1_v1_f1126_f1145_source_ownership_pc34(unsigned int number)
{
    if (number < 1126U || number > 1145U) return 0;
    return &kOwnership[number - 1126U];
}

int dm1_v1_f1126_f1145_admits_authentic_route_pc34(unsigned int number)
{
    const DM1_V1_F1126F1145OwnershipPc34 *entry =
        dm1_v1_f1126_f1145_source_ownership_pc34(number);
    return entry != 0 && entry->kind == DM1_V1_F1126_F1145_EXISTING_PC34_OWNER_PC34;
}

int dm1_v1_f1126_f1145_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1126_f1145_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB PALETTE.C:136-157; FILLBOX.C:6,37-41,738; BLIT.C:2036; "
           "CONVBOXC.C; AMIGINIT.C:550,668; COPERINT.C:9; AMIGAVID.C:32,50; "
           "CLIKCHAM.C F0367; CLIKMENU.C F0369; CLIKVIEW.C F0372-F0374. "
           "No generated UI, graphics, timing, palette, input, or Copper route.";
}
