#include "dm1_v1_f0261_f0280_movement_champion_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F0261F0280SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    entries = dm1_v1_f0261_f0280_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F0261F0280SourceAuditPc34 *found;
        if (entries[index].symbol_number != 261u + index ||
            !entries[index].raw_original_data_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_ui_or_party_mutation ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner) return 1;
        found = dm1_v1_f0261_f0280_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_f0261_f0280_source_audit_find_pc34(260u) ||
        dm1_v1_f0261_f0280_source_audit_find_pc34(281u) ||
        !strstr(dm1_v1_f0261_f0280_source_audit_evidence_pc34(), "does not mutate party")) return 1;
    puts("PASS: DM1 F0261-F0280 movement/champion source-owner audit");
    return 0;
}
