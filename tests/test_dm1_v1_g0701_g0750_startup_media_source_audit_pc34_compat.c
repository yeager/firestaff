#include "dm1_v1_g0701_g0750_startup_media_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0701G0750SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    unsigned int fail_closed_count = 0u;
    unsigned int source_material_count = 0u;

    entries = dm1_v1_g0701_g0750_source_audit_pc34(&count);
    if (!entries || count != 44u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0701G0750SourceAuditPc34 *found;
        if (!entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].independent_global_abi_forbidden) return 1;
        found = dm1_v1_g0701_g0750_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
        if (strstr(entries[index].firestaff_owner_or_fail_closed_boundary, "fail_closed")) {
            ++fail_closed_count;
        }
        if (entries[index].authentic_source_material_required) ++source_material_count;
    }
    if (fail_closed_count != 18u || source_material_count != 25u ||
        dm1_v1_g0701_g0750_source_audit_find_pc34(700u) ||
        dm1_v1_g0701_g0750_source_audit_find_pc34(714u) ||
        dm1_v1_g0701_g0750_source_audit_find_pc34(750u) ||
        !strstr(dm1_v1_g0701_g0750_source_audit_evidence_pc34(), "No independent global ABI")) return 1;
    puts("PASS: DM1 G0701-G0750 startup-media source-owner audit");
    return 0;
}
