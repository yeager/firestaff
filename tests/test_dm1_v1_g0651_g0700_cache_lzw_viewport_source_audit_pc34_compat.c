#include "dm1_v1_g0651_g0700_cache_lzw_viewport_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1_V1_G0651G0700SourceAuditPc34 *entries;
    size_t count;
    size_t index;
    unsigned int fail_closed_count = 0u;
    unsigned int dungeon_asset_count = 0u;
    unsigned int allocator_boundary_count = 0u;

    entries = dm1_v1_g0651_g0700_source_audit_pc34(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1_V1_G0651G0700SourceAuditPc34 *found;
        if (entries[index].symbol_number != 651u + index ||
            !entries[index].redmcsb_anchor ||
            !entries[index].firestaff_owner_or_fail_closed_boundary ||
            !entries[index].fail_closed_when_unavailable ||
            !entries[index].independent_global_abi_forbidden) return 1;
        found = dm1_v1_g0651_g0700_source_audit_find_pc34(entries[index].symbol_number);
        if (found != &entries[index]) return 1;
        if (strstr(entries[index].firestaff_owner_or_fail_closed_boundary, "fail_closed")) {
            ++fail_closed_count;
        }
        if (entries[index].owner_kind == DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34) {
            ++dungeon_asset_count;
            if (!entries[index].authentic_source_material_required) return 1;
        }
        if (entries[index].owner_kind == DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34) {
            ++allocator_boundary_count;
            if (!strstr(entries[index].firestaff_owner_or_fail_closed_boundary, "block-list")) return 1;
        }
    }
    if (fail_closed_count != 19u || dungeon_asset_count != 26u ||
        allocator_boundary_count != 5u ||
        dm1_v1_g0651_g0700_source_audit_find_pc34(650u) ||
        dm1_v1_g0651_g0700_source_audit_find_pc34(701u) ||
        !strstr(dm1_v1_g0651_g0700_source_audit_evidence_pc34(), "No independent global ABI")) return 1;
    puts("PASS: DM1 G0651-G0700 cache-LZW-viewport source-owner audit");
    return 0;
}
