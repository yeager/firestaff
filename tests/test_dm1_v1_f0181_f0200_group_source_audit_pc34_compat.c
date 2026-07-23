#include "dm1_v1_f0181_f0200_group_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0181F0200GroupSourceAuditPc34 *entries;
    size_t count;
    size_t index;
    entries = dm1_v1_f0181_f0200_group_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0181F0200GroupSourceAuditPc34 *found;
        if (entries[index].symbol_number != 181u + index ||
            !entries[index].raw_pc34_required ||
            !entries[index].fail_closed_when_unavailable ||
            !strstr(entries[index].redmcsb_anchor, "GROUP.C:") ||
            !entries[index].firestaff_owner) return 1;
        found = dm1_v1_f0181_f0200_group_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_f0181_f0200_group_source_audit_find_pc34(180u) ||
        dm1_v1_f0181_f0200_group_source_audit_find_pc34(201u) ||
        !strstr(dm1_v1_f0181_f0200_group_source_audit_evidence_pc34(), "fail closed")) return 1;
    puts("PASS: DM1 F0181-F0200 source-owner audit");
    return 0;
}
