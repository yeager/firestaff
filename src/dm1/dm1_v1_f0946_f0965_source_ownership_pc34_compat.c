#include "dm1_v1_f0946_f0965_source_ownership_pc34_compat.h"

static const DM1_V1_F0946F0965OwnershipPc34 kOwnership[] = {
    {946, DM1_V1_F0946_F0965_PLATFORM_BOUNDARY_PC34, "F0946_ReleaseAudioData", "SOUND.C:496-557", "Existing Amiga audio.device boundary; no PC34 audio substitute."},
    {947, DM1_V1_F0946_F0965_PLATFORM_BOUNDARY_PC34, "F0947_InitDiskData", "EXEC.C:547-566", "Existing Amiga trackdisk boundary; no PC34 disk substitute."},
    {948, DM1_V1_F0946_F0965_PLATFORM_BOUNDARY_PC34, "F0948_ReleaseDiskData", "EXEC.C:568-579", "Existing Amiga trackdisk boundary; no PC34 disk substitute."},
    {949, DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34, "F0949_JAPANESE_", "JAPANESE.C:15-34", "Existing source-locked PC-98 character conversion owner."},
    {950, DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34, "F0950_JAPANESE_", "JAPANESE.C:36-74", "Existing PC-98 I/O character-pattern owner; caller must supply real port route."},
    {951, DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34, "F0951_JAPANESE_GetCharacterPattern", "JAPANESE.C:76-93", "Existing source-locked A100 pattern owner; caller must supply real pattern data."},
    {952, DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34, "F0952_JAPANESE_Print", "JAPANESE.C:206-381", "Existing source-locked PC-98 raster owner; absent font/I-O data fails closed."},
    {953, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0953_ui_Count", "CHAMPION.C F0316", "Local scent storage; no standalone F route."},
    {954, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0954_i_ScentIndex", "CHAMPION.C F0317", "Local scent storage; no standalone F route."},
    {955, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0955_B_Merge", "CHAMPION.C F0317", "Local scent storage; no standalone F route."},
    {956, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0956_B_CycleCountDefined", "CHAMPION.C F0317", "Local scent storage; no standalone F route."},
    {957, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0957_ps_Scent", "CHAMPION.C F0317", "Local scent storage; no standalone F route."},
    {958, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0958_s_Scent", "CHAMPION.C F0317", "Local scent storage; no standalone F route."},
    {959, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0959_ui_Cell", "CHAMPION.C F0318", "Local drop storage; no standalone F route."},
    {960, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0960_T_Thing", "CHAMPION.C F0318", "Local drop storage; no standalone F route."},
    {961, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0961_ui_SlotIndex", "CHAMPION.C F0318", "Local drop storage; no standalone F route."},
    {962, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0962_ui_Multiple", "CHAMPION.C F0319", "Local kill storage; no standalone F route."},
    {963, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0963_ui_AliveChampionIndex", "CHAMPION.C F0319", "Local kill storage; no standalone F route."},
    {964, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0964_T_Thing", "CHAMPION.C F0319", "Local kill storage; no standalone F route."},
    {965, DM1_V1_F0946_F0965_LOCAL_ONLY_PC34, "L0965_ps_Champion", "CHAMPION.C F0319", "Local kill storage; no standalone F route."},
};

const DM1_V1_F0946F0965OwnershipPc34 *
dm1_v1_f0946_f0965_source_ownership_pc34(unsigned int number) {
    if (number < 946U || number > 965U) return 0;
    return &kOwnership[number - 946U];
}

int dm1_v1_f0946_f0965_has_synthetic_route_pc34(unsigned int number) {
    (void)number;
    return 0;
}

const char *dm1_v1_f0946_f0965_source_ownership_evidence_pc34(void) {
    return "ReDMCSB SOUND.C:496-557; EXEC.C:547-579; JAPANESE.C:15-381; "
           "REDMCSB label inventory L0953-L0965. No generated UI, graphics, timing, or font route.";
}
