#include "dm1_v1_g1000_g1050_viewport_input_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G1000G1050SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    unsigned int fail_closed_count = 0u;
    unsigned int authentic_material_count = 0u;

    entries = dm1_v1_g1000_g1050_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G1000G1050SourceAuditPc34 *found =
            dm1_v1_g1000_g1050_source_audit_find_pc34(entries[index].symbol_number);
        if (!entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].independent_global_abi_forbidden || found != &entries[index]) return 1;
        if (strstr(entries[index].firestaff_owner_or_fail_closed_boundary, "fail_closed")) {
            ++fail_closed_count;
        }
        if (entries[index].authentic_source_material_required) ++authentic_material_count;
    }
    if (fail_closed_count != 26u || authentic_material_count != 8u ||
        dm1_v1_g1000_g1050_source_audit_find_pc34(999u) ||
        dm1_v1_g1000_g1050_source_audit_find_pc34(1017u) ||
        dm1_v1_g1000_g1050_source_audit_find_pc34(1051u) ||
        !strstr(dm1_v1_g1000_g1050_source_audit_evidence_pc34(), "G1017 is absent") ||
        !strstr(dm1_v1_g1000_g1050_source_audit_evidence_pc34(), "No independent global ABI")) return 1;
    puts("PASS: DM1 G1000-G1050 viewport-input source-owner audit");
    return 0;
}
