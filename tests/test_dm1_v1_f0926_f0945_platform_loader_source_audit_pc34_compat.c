#include "dm1_v1_f0926_f0945_platform_loader_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0926F0945SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    size_t direct_owner_count = 0u;

    entries = dm1_v1_f0926_f0945_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0926F0945SourceAuditPc34 *found;
        if (entries[index].symbol_number != 926u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_ui_or_timing ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        if (entries[index].direct_redmcsb_source_owner) ++direct_owner_count;
        found = dm1_v1_f0926_f0945_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (direct_owner_count != 18u ||
        dm1_v1_f0926_f0945_source_audit_find_pc34(925u) ||
        dm1_v1_f0926_f0945_source_audit_find_pc34(946u) ||
        !strstr(dm1_v1_f0926_f0945_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 F0926-F0945 platform/loader source-owner audit");
    return 0;
}
