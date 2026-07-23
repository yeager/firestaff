#include "dm1_v1_f0931_f0934_prim_checksum_hex_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0931F0934SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_f0931_f0934_source_audit_pc34(&count);
    if (!entries || count != 4u) return 1;

    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0931F0934SourceAuditPc34 *found;

        if (entries[index].symbol_number != 931u + index ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner ||
            !entries[index].direct_pc34_owner ||
            !entries[index].fail_closed_when_source_contract_is_unavailable ||
            !entries[index].audit_only_no_synthetic_wrapper) return 1;
        found = dm1_v1_f0931_f0934_source_audit_find_pc34(
            entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }

    if (dm1_v1_f0931_f0934_source_audit_find_pc34(930u) ||
        dm1_v1_f0931_f0934_source_audit_find_pc34(935u) ||
        !strstr(dm1_v1_f0931_f0934_source_audit_evidence_pc34(),
                "no synthetic wrapper")) return 1;

    puts("PASS: DM1 F0931-F0934 PRIM checksum/hex existing-owner audit");
    return 0;
}
