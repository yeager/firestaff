#include "dm1_v1_f1226_f1245_anim_audio_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_F1226F1245SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    size_t direct_owner_count = 0u;

    entries = dm1_v1_f1226_f1245_source_audit_pc34(&count);
    if (!entries || count != 20u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_F1226F1245SourceAuditPc34 *found;
        if (entries[index].symbol_number != 1226u + index ||
            !entries[index].raw_source_or_pc34_material_required ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].audit_only_no_synthetic_render_ui_or_timing ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary) return 1;
        if (entries[index].direct_redmcsb_source_owner) ++direct_owner_count;
        found = dm1_v1_f1226_f1245_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
    }
    if (direct_owner_count != 19u ||
        dm1_v1_f1226_f1245_source_audit_find_pc34(1225u) ||
        dm1_v1_f1226_f1245_source_audit_find_pc34(1246u) ||
        !strstr(dm1_v1_f1226_f1245_source_audit_evidence_pc34(), "does not render")) return 1;
    puts("PASS: DM1 F1226-F1245 ANIM/audio source-owner audit");
    return 0;
}
