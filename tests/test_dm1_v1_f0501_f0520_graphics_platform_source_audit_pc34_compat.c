#include "dm1_v1_f0501_f0520_graphics_platform_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0501F0520SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_f0501_f0520_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0501F0520SourceAuditPc34 *found;
        if (entries[index].symbol_number != 501u + index ||
            !entries[index].raw_original_data_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_or_ui ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner) return 1;
        found = dm1_v1_f0501_f0520_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_f0501_f0520_source_audit_find_pc34(500u) ||
        dm1_v1_f0501_f0520_source_audit_find_pc34(521u) ||
        !strstr(dm1_v1_f0501_f0520_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 F0501-F0520 graphics/platform source-owner audit");
    return 0;
}
