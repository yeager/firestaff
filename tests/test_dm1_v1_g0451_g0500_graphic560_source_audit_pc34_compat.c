#include "dm1_v1_g0451_g0500_graphic560_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0451G0500SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_g0451_g0500_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0451G0500SourceAuditPc34 *found;
        if (entries[index].symbol_number != 451u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_behavior) return 1;
        found = dm1_v1_g0451_g0500_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_g0451_g0500_source_audit_find_pc34(450u) ||
        dm1_v1_g0451_g0500_source_audit_find_pc34(501u) ||
        !strstr(entries[34].firestaff_owner_or_fail_closed_boundary, "graphic560") ||
        !strstr(entries[49].firestaff_owner_or_fail_closed_boundary, "graphic560") ||
        !strstr(dm1_v1_g0451_g0500_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 G0451-G0500 Graphic560 source-owner audit");
    return 0;
}
