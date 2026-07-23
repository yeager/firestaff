#include "dm1_v1_f0931_f0934_prim_checksum_hex_source_audit_pc34_compat.h"

static const DM1_V1_F0931F0934SourceAuditPc34 k_audit[] = {
    { 931u, "PRIM1.C; PRIM.H F0931_ChecksumWords",
      "f0931_checksum_words_pc34_compat", 1, 1, 1 },
    { 932u, "PRIM1.C; PRIM.H F0932_ChecksumBytes",
      "f0932_checksum_bytes_pc34_compat", 1, 1, 1 },
    { 933u, "PRIM1.C; PRIM.H; STRING.C F0933_GetHexStringFromValue",
      "F0933_GetHexStringFromValue", 1, 1, 1 },
    { 934u, "PRIM1.C; PRIM.H; STRING.C F0934_ConvertValueToHexDigits",
      "redmcsb_f0934_convert_value_to_hex_digits", 1, 1, 1 }
};

const DM1_V1_F0931F0934SourceAuditPc34 *
dm1_v1_f0931_f0934_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0931F0934SourceAuditPc34 *
dm1_v1_f0931_f0934_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0931_f0934_source_audit_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C, PRIM.H, and STRING.C own F0931-F0934. "
           "The PC34 checksum and uppercase hexadecimal conversions are bound "
           "to their existing Firestaff owners only. This audit adds no "
           "synthetic wrapper, UI route, or fallback implementation.";
}
