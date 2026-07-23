#include "dm1_v1_g0601_g0650_mouse_graphics_memory_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0601G0650SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    unsigned int platform_count = 0u;
    unsigned int graphics_count = 0u;

    entries = dm1_v1_g0601_g0650_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0601G0650SourceAuditPc34 *found;
        if (entries[index].symbol_number != 601u + index ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_platform_boundary ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].independent_global_abi_forbidden) return 1;
        found = dm1_v1_g0601_g0650_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
        if (entries[index].owner_kind == DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34) {
            ++platform_count;
            if (!strstr(entries[index].firestaff_owner_or_platform_boundary, "platform_boundary")) return 1;
        }
        if (entries[index].owner_kind == DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34) {
            ++graphics_count;
            if (!entries[index].authentic_source_material_required) return 1;
        }
    }
    if (platform_count != 9u || graphics_count != 13u ||
        dm1_v1_g0601_g0650_source_audit_find_pc34(600u) ||
        dm1_v1_g0601_g0650_source_audit_find_pc34(651u) ||
        !strstr(dm1_v1_g0601_g0650_source_audit_evidence_pc34(), "No independent global ABI")) return 1;
    puts("PASS: DM1 G0601-G0650 mouse-graphics-memory source-owner audit");
    return 0;
}
