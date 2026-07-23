#include "dm1_v1_f0886_f0905_source_ownership_pc34_compat.h"

static const DM1_V1_F0886F0905OwnershipPc34 kOwnership[] = {
    {886, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0886_pT_Thing", "CHAMDRAW.C F0296", "Local object-icon storage; no standalone PC34 render route."},
    {887, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0887_ps_Champion", "CHAMDRAW.C F0296", "Local object-icon storage; no standalone PC34 render route."},
    {888, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0888_i_IconIndex", "CHAMDRAW.C F0296", "Local object-icon storage; no standalone PC34 render route."},
    {889, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0889_ui_ObjectIconChanged", "CHAMDRAW.C F0296", "Local object-icon storage; no standalone PC34 render route."},
    {890, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0890_T_LeaderHandObject", "CHAMPION.C F0298", "Local leader-hand storage; no standalone PC34 UI route."},
    {891, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0891_i_Multiple", "CHAMPION.C F0299", "Local modifier storage; no standalone PC34 route."},
    {892, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0892_i_Modifier", "CHAMPION.C F0299", "Local modifier storage; no standalone PC34 route."},
    {893, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0893_ps_Weapon", "CHAMPION.C F0299", "Local weapon storage; no standalone PC34 route."},
    {894, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0894_T_Thing", "CHAMPION.C F0300", "Local slot-removal storage; no standalone PC34 UI route."},
    {895, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0895_i_IconIndex", "CHAMPION.C F0300", "Local slot-removal storage; no standalone PC34 UI route."},
    {896, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0896_ps_Champion", "CHAMPION.C F0300", "Local slot-removal storage; no standalone PC34 UI route."},
    {897, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0897_ps_Weapon", "CHAMPION.C F0300", "Local slot-removal storage; no standalone PC34 UI route."},
    {898, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0898_B_IsInventoryChampion", "CHAMPION.C F0300", "Local slot-removal storage; no standalone PC34 UI route."},
    {899, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0899_i_IconIndex", "CHAMPION.C F0301", "Local slot-addition storage; no standalone PC34 UI route."},
    {900, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0900_ps_Champion", "CHAMPION.C F0301", "Local slot-addition storage; no standalone PC34 UI route."},
    {901, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0901_ps_Weapon", "CHAMPION.C F0301", "Local slot-addition storage; no standalone PC34 UI route."},
    {902, DM1_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0902_DrawFTLLogo", "SWSH.C:357; SWSHIIGS.C; dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat", "Authenticated original SWSH/SWSHIIGS frame, palette, and timing owner; L0902 remains a separate local."},
    {903, DM1_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0903_DrawErrorMessage", "SWSH.C:2208-2215; redmcsb_f0903_draw_error_message_pc34_compat", "Original G0747 four-plane copy owner; absent planes fail closed; L0903 remains a separate local."},
    {904, DM1_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34, "F0904_PaletteAnimation", "SWSH.C:2902-2919; dm1_v1_f0904_palette_animation_pc34_compat", "Caller must supply original packed palette commands; unterminated or absent source fails closed; L0904 remains a separate local."},
    {905, DM1_V1_F0886_F0905_LOCAL_ONLY_PC34, "L0905_T_LeaderHandObject", "CHAMPION.C F0302", "Local slot-click storage; no standalone PC34 UI route."},
};

const DM1_V1_F0886F0905OwnershipPc34 *
dm1_v1_f0886_f0905_source_ownership_pc34(unsigned int number) {
    if (number < 886U || number > 905U) return 0;
    return &kOwnership[number - 886U];
}

int dm1_v1_f0886_f0905_has_standalone_synthetic_route_pc34(unsigned int number) {
    (void)number;
    return 0;
}

const char *dm1_v1_f0886_f0905_source_ownership_evidence_pc34(void) {
    return "ReDMCSB label inventory L0886-L0905; SWSH.C:357,2208-2215,2902-2919; "
           "authenticated F0902 media, F0903 original bitplanes, and F0904 original palette commands only.";
}
