#include "dm1_v1_f0949_f0952_japanese_existing_owner_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_f0949_f0952_existing_owner_audit_pc34(&count);
    if (!entries || count != 4u) return 1;

    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *found;

        if (entries[index].symbol_number != 949u + index ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner ||
            !entries[index].direct_source_owner ||
            !entries[index].audit_only_no_synthetic_wrapper) return 1;
        found = dm1_v1_f0949_f0952_existing_owner_audit_find_pc34(
            entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }

    if (entries[0].requires_real_pc98_material_or_io ||
        !entries[1].requires_real_pc98_material_or_io ||
        !entries[2].requires_real_pc98_material_or_io ||
        !entries[3].requires_real_pc98_material_or_io ||
        dm1_v1_f0949_f0952_existing_owner_audit_find_pc34(948u) ||
        dm1_v1_f0949_f0952_existing_owner_audit_find_pc34(953u) ||
        !strstr(dm1_v1_f0949_f0952_existing_owner_audit_evidence_pc34(),
                "no synthetic wrapper")) return 1;

    puts("PASS: DM1 F0949-F0952 Japanese existing-owner audit");
    return 0;
}
