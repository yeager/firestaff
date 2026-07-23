#include "dm1_v1_g0301_g0350_base_runtime_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0301G0350SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_g0301_g0350_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0301G0350SourceAuditPc34 *found;
        if (entries[index].symbol_number != 301u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_behavior ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        found = dm1_v1_g0301_g0350_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_g0301_g0350_source_audit_find_pc34(300u) ||
        dm1_v1_g0301_g0350_source_audit_find_pc34(351u) ||
        !strstr(dm1_v1_g0301_g0350_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 G0301-G0350 base-runtime source-owner audit");
    return 0;
}
