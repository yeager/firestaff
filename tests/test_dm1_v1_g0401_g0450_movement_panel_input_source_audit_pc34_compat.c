#include "dm1_v1_g0401_g0450_movement_panel_input_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0401G0450SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_g0401_g0450_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0401G0450SourceAuditPc34 *found;
        if (entries[index].symbol_number != 401u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_behavior ||
            !strstr(entries[index].firestaff_owner_or_fail_closed_boundary, "fail_closed")) return 1;
        found = dm1_v1_g0401_g0450_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_g0401_g0450_source_audit_find_pc34(400u) ||
        dm1_v1_g0401_g0450_source_audit_find_pc34(451u) ||
        !strstr(dm1_v1_g0401_g0450_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 G0401-G0450 movement-panel-input source-owner audit");
    return 0;
}
