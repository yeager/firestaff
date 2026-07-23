#include "dm1_v1_f1746_f1785_debug_error_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F1746F1785SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    size_t direct_owner_count = 0u;

    entries = dm1_v1_f1746_f1785_source_audit_pc34(&count);
    if (!entries || count != 40u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F1746F1785SourceAuditPc34 *found;
        if (entries[index].symbol_number != 1746u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_ui_or_timing ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        if (entries[index].direct_redmcsb_source_owner) ++direct_owner_count;
        found = dm1_v1_f1746_f1785_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (direct_owner_count != 20u ||
        dm1_v1_f1746_f1785_source_audit_find_pc34(1745u) ||
        dm1_v1_f1746_f1785_source_audit_find_pc34(1786u) ||
        !strstr(dm1_v1_f1746_f1785_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 F1746-F1785 debug/error source-owner audit");
    return 0;
}
