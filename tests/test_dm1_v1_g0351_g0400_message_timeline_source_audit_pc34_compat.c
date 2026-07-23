#include "dm1_v1_g0351_g0400_message_timeline_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0351G0400SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_g0351_g0400_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0351G0400SourceAuditPc34 *found;
        if (entries[index].symbol_number != 351u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_behavior ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        found = dm1_v1_g0351_g0400_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_g0351_g0400_source_audit_find_pc34(350u) ||
        dm1_v1_g0351_g0400_source_audit_find_pc34(401u) ||
        !strstr(entries[4].firestaff_owner_or_fail_closed_boundary, "text_message") ||
        !strstr(entries[18].firestaff_owner_or_fail_closed_boundary, "event_timer") ||
        !strstr(entries[26].firestaff_owner_or_fail_closed_boundary, "g0377") ||
        !strstr(dm1_v1_g0351_g0400_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 G0351-G0400 message-timeline source-owner audit");
    return 0;
}
