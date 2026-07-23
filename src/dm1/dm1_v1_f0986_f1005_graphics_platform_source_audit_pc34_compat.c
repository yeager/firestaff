#include "dm1_v1_f0986_f1005_graphics_platform_source_audit_pc34_compat.h"

static const DM1_V1_F0986F1005SourceAuditPc34 k_audit[] = {
    { 986u, "no numbered F0986 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 987u, "no numbered F0987 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 988u, "no numbered F0988 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 989u, "no numbered F0989 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 990u, "no numbered F0990 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 991u, "no numbered F0991 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 992u, "no numbered F0992 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 993u, "no numbered F0993 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 994u, "no numbered F0994 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 995u, "no numbered F0995 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 996u, "no numbered F0996 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 997u, "no numbered F0997 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 998u, "no numbered F0998 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 999u, "no numbered F0999 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1000u, "DUNVIEW.C:2063 F1000_ flipped wall bitmap preparation", "dm1_v1_viewport_f0099_row_local_flip_pc34_compat", 1, 1, 1, 1 },
    { 1001u, "JAPANESE.C:97 F1001_JAPANESE_LoadANKCharacterPatterns", "redmcsb_f1001_japanese_load_ank_character_patterns_pc34_compat", 1, 1, 1, 1 },
    { 1002u, "BASE.C:1202 F1002_Call_F0132_VIDEO_Blit", "redmcsb_f1002_call_f0132_video_blit", 1, 1, 1, 1 },
    { 1003u, "IMAGE4.C:114 F1003_ packed pixel copy", "image_backend_pc34_compat", 1, 1, 1, 1 },
    { 1004u, "BLTSHRNK.C:1557 F1004_VIDEO_BlitShrinkWithPaletteChanges", "redmcsb_f1004_video_blit_shrink_with_palette_changes_pc34_compat", 1, 1, 1, 1 },
    { 1005u, "DUNVIEW.C:3250 F1005_ derived bitmap selection", "fail_closed: static derived-bitmap route lacks standalone PC34 owner", 1, 1, 1, 1 }
};

const DM1_V1_F0986F1005SourceAuditPc34 *
dm1_v1_f0986_f1005_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0986F1005SourceAuditPc34 *
dm1_v1_f0986_f1005_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0986_f1005_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C, JAPANESE.C, BASE.C, IMAGE4.C, and BLTSHRNK.C "
           "are the authority for F0986-F1005. F0986-F0999 have no numbered "
           "source body in the audited corpus. PC-98-only and unproven derived "
           "bitmap routes remain fail closed without authentic PC34 material. "
           "The audit does not render or synthesize UI or timing paths.";
}
