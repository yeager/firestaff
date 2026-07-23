#include "dm1_v1_g0501_g0550_graphic560_save_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0501G0550SourceAuditPc34 *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_g0501_g0550_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0501G0550SourceAuditPc34 *found;
        if (entries[index].symbol_number != 501u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_behavior) return 1;
        found = dm1_v1_g0501_g0550_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (dm1_v1_g0501_g0550_source_audit_find_pc34(500u) ||
        dm1_v1_g0501_g0550_source_audit_find_pc34(551u) ||
        !strstr(entries[0].firestaff_owner_or_fail_closed_boundary, "graphic560") ||
        !strstr(entries[24].firestaff_owner_or_fail_closed_boundary, "dungeon_decompressor") ||
        !strstr(entries[33].firestaff_owner_or_fail_closed_boundary, "save_load") ||
        !strstr(dm1_v1_g0501_g0550_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 G0501-G0550 Graphic560-save source-owner audit");
    return 0;
}
