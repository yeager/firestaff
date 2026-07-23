#include "dm1_v1_f0949_f0952_japanese_existing_owner_audit_pc34_compat.h"

static const DM1_V1_F0949F0952ExistingOwnerAuditPc34 k_audit[] = {
    { 949u, "JAPANESE.C:15-34 F0949_JAPANESE_",
      "redmcsb_f0949_japanese_pc34_compat", 1, 0, 1 },
    { 950u, "JAPANESE.C:36-74 F0950_JAPANESE_",
      "redmcsb_f0950_japanese_character_pattern_pc34_compat", 1, 1, 1 },
    { 951u, "JAPANESE.C:76-93 F0951_JAPANESE_GetCharacterPattern",
      "redmcsb_f0951_japanese_get_character_pattern", 1, 1, 1 },
    { 952u, "JAPANESE.C:270-381 F0952_JAPANESE_Print",
      "redmcsb_f0952_japanese_print_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *
dm1_v1_f0949_f0952_existing_owner_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *
dm1_v1_f0949_f0952_existing_owner_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0949_f0952_existing_owner_audit_evidence_pc34(void)
{
    return "ReDMCSB JAPANESE.C:15-381 owns F0949-F0952. The existing "
           "Firestaff Shift-JIS conversion, PC-98 character-pattern, ANK "
           "segment, and Japanese raster owners are bound directly; PC-98 "
           "I/O and font material remain caller-supplied. This audit adds no "
           "synthetic wrapper, UI, or fallback behavior.";
}
