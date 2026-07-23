#include "csb_v1_f0986_f1005_graphics_source_boundary_pc34_compat.h"

#include <string.h>

typedef struct {
    CSB_V1_F0986F1005SourceKindPc34 source_kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} CSB_V1_F0986F1005SourceSpecPc34;

static const CSB_V1_F0986F1005SourceSpecPc34 kSpecs[] = {
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0986_i_DamagedChampionCount", "CHAMPION.C:2004 F0324", "Function-local champion state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0987_pc_Unreferenced", "CHAMPION.C:2000 F0324", "Function-local champion state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0988_i_Stamina", "CHAMPION.C:2033 F0325", "Function-local champion state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0989_ps_Champion", "CHAMPION.C:2032 F0325", "Function-local champion state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0990_ui_Direction", "CHAMPION.C:2061 F0326", "Function-local projectile state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0991_i_StepEnergy", "CHAMPION.C:2085 F0327", "Function-local spell state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0992_ps_Champion", "CHAMPION.C:2083 F0327", "Function-local spell state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0993_i_KineticEnergy", "CHAMPION.C:2123 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0994_i_Multiple", "CHAMPION.C:2124 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0995_i_Multiple", "CHAMPION.C:2128 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0996_T_Thing", "CHAMPION.C:2133 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0997_ps_Champion", "CHAMPION.C:2120 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0998_ps_WeaponInfo", "CHAMPION.C:2121 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_LOCAL_LABEL_PC34, "L0999_T_ActionHandThing", "CHAMPION.C:2142 F0328", "Function-local throw state; no standalone CSB route."},
    {CSB_V1_F0986_F1005_UNNUMBERED_SOURCE_HELPER_PC34, "F1000_", "DUNVIEW.C:2063", "Unnumbered viewport helper; no authenticated CSB PC34 owner."},
    {CSB_V1_F0986_F1005_NON_PC34_MEDIA_PC34, "F1001_JAPANESE_LoadANKCharacterPatterns", "JAPANESE.C:97-205", "P20JA/P20JB PC-98 media route; not PC34."},
    {CSB_V1_F0986_F1005_EXISTING_OWNER_WITHOUT_CSB_ADMISSION_PC34, "F1002_Call_F0132_VIDEO_Blit", "BASE.C:1202-1212; csb_v1_f1002_f1032_f1033_f1052_video_helpers_pc34_compat", "Existing helper remains exclusive; no authenticated CSB PC34 bitmap/palette admission is proven."},
    {CSB_V1_F0986_F1005_UNNUMBERED_SOURCE_HELPER_PC34, "F1003_", "IMAGE4.C:114", "Unnumbered packed-pixel helper; no authenticated CSB PC34 owner."},
    {CSB_V1_F0986_F1005_NON_PC34_MEDIA_PC34, "F1004_VIDEO_BlitShrinkWithPaletteChanges", "BLTSHRNK.C:1556-1595", "P20JA/P20JB PC-98 media route; not PC34."},
    {CSB_V1_F0986_F1005_UNNUMBERED_SOURCE_HELPER_PC34, "F1005_", "DUNVIEW.C:3250", "Unnumbered derived-bitmap helper; no authenticated CSB PC34 owner."},
};

int csb_v1_f0986_f1005_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F0986F1005SourceBoundaryReceiptPc34 *out_receipt)
{
    CSB_V1_F0986F1005SourceBoundaryReceiptPc34 receipt;
    const CSB_V1_F0986F1005SourceSpecPc34 *spec;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (function_number < 986u || function_number > 1005u) {
        return 0;
    }

    spec = &kSpecs[function_number - 986u];
    receipt.function_number = function_number;
    receipt.source_kind = spec->source_kind;
    receipt.symbol = spec->symbol;
    receipt.source_anchor = spec->source_anchor;
    receipt.owner_or_rationale = spec->owner_or_rationale;
    receipt.authentic_pc34_material_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out_receipt = receipt;
    return 0;
}

const char *csb_v1_f0986_f1005_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C, DUNVIEW.C, JAPANESE.C, BASE.C, IMAGE4.C, and "
           "BLTSHRNK.C F0986-F1005: no authenticated CSB PC34 bitmap/palette "
           "consumer is proven, so all runtime routes fail closed.";
}
