#include "dm1_v1_p0866_p0885_parameter_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_P0866P0885SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_p0866_p0885_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_P0866P0885SourceAuditPc34 *found;
        if (entries[index].parameter_number != 866u + index ||
            !entries[index].direct_redmcsb_parameter ||
            !entries[index].pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_ui_or_timing ||
            !entries[index].redmcsb_parameter_and_owner ||
            !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        found = dm1_v1_p0866_p0885_source_audit_find_pc34(entries[index].parameter_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_p0866_p0885_source_audit_find_pc34(865u) ||
        dm1_v1_p0866_p0885_source_audit_find_pc34(886u) ||
        !strstr(dm1_v1_p0866_p0885_source_audit_evidence_pc34(), "not F functions") ||
        !strstr(dm1_v1_p0866_p0885_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 P0866-P0885 parameter source-owner audit");
    return 0;
}
