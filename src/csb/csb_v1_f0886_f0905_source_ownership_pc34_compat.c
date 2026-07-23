#include "csb_v1_f0886_f0905_source_ownership_pc34_compat.h"

static const CSB_V1_F0886F0905OwnershipPc34 kOwnership[] = {
    {886u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0886_pT_Thing",
     "CHAMDRAW.C:1191 F0296_CHAMPION_DrawChangedObjectIcons",
     "Function-local object-icon storage; no standalone CSB PC34 route."},
    {887u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0887_ps_Champion",
     "CHAMDRAW.C:1192 F0296_CHAMPION_DrawChangedObjectIcons",
     "Function-local champion storage; no standalone CSB PC34 route."},
    {888u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0888_i_IconIndex",
     "CHAMDRAW.C:1202 F0296_CHAMPION_DrawChangedObjectIcons",
     "Function-local icon-index storage; no standalone CSB PC34 route."},
    {889u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0889_ui_ObjectIconChanged",
     "CHAMDRAW.C:1207 F0296_CHAMPION_DrawChangedObjectIcons",
     "Function-local object-icon flag; no standalone CSB PC34 route."},
    {890u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0890_T_LeaderHandObject",
     "CHAMPION.C:276 F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
     "Function-local leader-hand storage; no standalone CSB PC34 route."},
    {891u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0891_i_Multiple",
     "CEDT004.C:267 F0299_CHAMPION_ApplyObjectModifiersToStatistics",
     "Function-local modifier storage; no standalone CSB PC34 route."},
    {892u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0892_i_Modifier",
     "CEDT004.C:262 F0299_CHAMPION_ApplyObjectModifiersToStatistics",
     "Function-local modifier storage; no standalone CSB PC34 route."},
    {893u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0893_ps_Weapon",
     "CEDT004.C:260 F0299_CHAMPION_ApplyObjectModifiersToStatistics",
     "Function-local weapon storage; no standalone CSB PC34 route."},
    {894u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0894_T_Thing",
     "CHAMPION.C:501 F0300_CHAMPION_GetObjectRemovedFromSlot",
     "Function-local slot-removal storage; no standalone CSB PC34 route."},
    {895u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0895_i_IconIndex",
     "CHAMPION.C:502 F0300_CHAMPION_GetObjectRemovedFromSlot",
     "Function-local slot-removal storage; no standalone CSB PC34 route."},
    {896u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0896_ps_Champion",
     "CHAMPION.C:499 F0300_CHAMPION_GetObjectRemovedFromSlot",
     "Function-local slot-removal storage; no standalone CSB PC34 route."},
    {897u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0897_ps_Weapon",
     "CHAMPION.C:500 F0300_CHAMPION_GetObjectRemovedFromSlot",
     "Function-local slot-removal storage; no standalone CSB PC34 route."},
    {898u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0898_B_IsInventoryChampion",
     "CHAMPION.C:504 F0300_CHAMPION_GetObjectRemovedFromSlot",
     "Function-local inventory flag; no standalone CSB PC34 route."},
    {899u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0899_i_IconIndex",
     "CHAMPION.C:597 F0301_CHAMPION_AddObjectInSlot",
     "Function-local slot-addition storage; no standalone CSB PC34 route."},
    {900u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0900_ps_Champion",
     "CHAMPION.C:595 F0301_CHAMPION_AddObjectInSlot",
     "Function-local slot-addition storage; no standalone CSB PC34 route."},
    {901u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0901_ps_Weapon",
     "CHAMPION.C:596 F0301_CHAMPION_AddObjectInSlot",
     "Function-local slot-addition storage; no standalone CSB PC34 route."},
    {902u, CSB_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0902_DrawFTLLogo",
     "SWSH.C:357; csb_v1_f0902_draw_ftl_logo_pc34_compat",
     "Existing CSB owner requires caller-bound original 320x200 logo and palette."},
    {903u, CSB_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0903_DrawErrorMessage",
     "SWSH.C:2208-2215; redmcsb_f0903_draw_error_message_pc34_compat",
     "Existing four-original-plane copy owner; absent source planes fail closed."},
    {904u, CSB_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0904_PaletteAnimation",
     "SWSH.C:1104; csb_v1_f0904_swsh_palette_animation_pc34_compat",
     "Existing CSB owner requires the original 27 two-word palette records."},
    {905u, CSB_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0905_T_LeaderHandObject",
     "CHAMPION.C:673 F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
     "Function-local click storage; no standalone CSB PC34 route."},
};

const CSB_V1_F0886F0905OwnershipPc34 *
csb_v1_f0886_f0905_source_ownership_pc34(unsigned int number)
{
    if (number < 886u || number > 905u) {
        return 0;
    }
    return &kOwnership[number - 886u];
}

int csb_v1_f0886_f0905_has_standalone_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *csb_v1_f0886_f0905_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB label inventory L0886-L0905 and SWSH.C F0902-F0904; "
           "only existing original-logo, error-plane, and palette owners are "
           "admitted, with no standalone synthetic CSB route.";
}
