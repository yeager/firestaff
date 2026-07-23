#include "dm1_v1_f1626_f1645_source_ownership_pc34_compat.h"

static const DM1_V1_F1626F1645OwnershipPc34 kOwnership[] = {
    {1626, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1626_VDI_vsc_form", "UTSTVDI3.C:124", "Atari VDI cursor-form route has no authenticated PC34 host receipt."},
    {1627, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1627_VDI_vex_timv", "UTSTVDI3.C:135", "Atari VDI timer-vector route has no authenticated PC34 host receipt."},
    {1628, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1628_VDI_v_show_c", "IO.C:4222; UTSTVDI3.C", "Atari VDI cursor-show route has no PC34 input/pointer owner."},
    {1629, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1629_VDI_v_hide_c", "IO.C:4214; UTSTVDI3.C", "Atari VDI cursor-hide route has no PC34 input/pointer owner."},
    {1630, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1630_VDI_vq_mouse", "IO.C:4250; UTSTVDI3.C", "Atari VDI mouse query has no caller-owned PC34 route."},
    {1631, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1631_VDI_vex_butv", "IO.C:4258; UTSTVDI3.C", "Atari VDI button callback has no PC34 route."},
    {1632, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1632_VDI_vex_motv", "IO.C:4254; UTSTVDI3.C", "Atari VDI motion callback has no PC34 route."},
    {1633, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1633_VDI_vex_curv", "UTSTVDI3.C:194", "Atari VDI cursor callback has no PC34 route."},
    {1634, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1634_VDI_vq_key_s", "UTSTVDI3.C:204", "Atari VDI keyboard-state query has no PC34 route."},
    {1635, DM1_V1_F1626_F1645_PLATFORM_BOUNDARY_PC34, "F1635_VDI_v_curhome", "UTSTVDI3.C:213", "Atari VDI cursor-home route has no PC34 route."},
    {1636, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1636_pui_SectorBuffer", "COPYPRO5.C F0210", "Function-local copy-protection storage; no standalone F1636 route."},
    {1637, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1637_i_Temp", "PROJEXPL.C F0227", "Function-local line-of-sight storage; no standalone F1637 route."},
    {1638, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1638_ui_MovementSoundIndex", "MOVESENS.C F0267", "Function-local move-sensor storage; no standalone F1638 route."},
    {1639, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1639_puc_Bitmap_Portrait_ChipMemory", "REVIVE.C F0280", "Function-local portrait storage; no standalone F1639 route."},
    {1640, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1640_ai_Box", "CHAMDRAW.C F0287", "Function-local champion-bar storage; no standalone F1640 route."},
    {1641, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1641_i_DestinationHeight", "CHAMDRAW.C F0291", "Function-local slot-draw storage; no standalone F1641 route."},
    {1642, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1642_puc_Bitmap_Portrait_ChipMemory", "PANEL.C F0354", "Function-local panel portrait storage; no standalone F1642 route."},
    {1643, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1643_i_Width", "ACTIDRAW.C F0385", "Function-local action-damage storage; no standalone F1643 route."},
    {1644, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1644_l_", "LOADSAVE.C F0433", "Function-local save storage; no standalone F1644 route."},
    {1645, DM1_V1_F1626_F1645_LOCAL_SYMBOL_PC34, "L1645_i_", "LOADSAVE.C F0433", "Function-local save storage; no standalone F1645 route."},
};

const DM1_V1_F1626F1645OwnershipPc34 *
dm1_v1_f1626_f1645_source_ownership_pc34(unsigned int number)
{
    if (number < 1626U || number > 1645U) return 0;
    return &kOwnership[number - 1626U];
}

int dm1_v1_f1626_f1645_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1626_f1645_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1626_f1645_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB UTSTVDI3.C:124-213; IO.C:4214-4258; COPYPRO5.C F0210; "
           "PROJEXPL.C F0227; MOVESENS.C F0267; REVIVE.C F0280; CHAMDRAW.C "
           "F0287/F0291; PANEL.C F0354; ACTIDRAW.C F0385; LOADSAVE.C F0433. "
           "No generated cursor, input, graphics, timing, UI, or save route.";
}
