#include "csb_v1_f2326_f2365_unowned_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const CSB_V1_F2326F2365SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = csb_v1_f2326_f2365_source_audit_pc34(&count);
    if (!entries || count != 40u) return 1;
    for (index = 0u; index < count; ++index) {
        const CSB_V1_F2326F2365SourceAuditPc34 *found;

        if (entries[index].symbol_number != 2326u + index ||
            entries[index].direct_redmcsb_source_owner ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_ui_or_timing ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        found = csb_v1_f2326_f2365_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (csb_v1_f2326_f2365_source_audit_find_pc34(2325u) ||
        csb_v1_f2326_f2365_source_audit_find_pc34(2366u) ||
        !strstr(csb_v1_f2326_f2365_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: CSB F2326-F2365 unowned source audit");
    return 0;
}
