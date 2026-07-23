#include "dm1_v1_g0651_g0700_cache_lzw_viewport_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner, kind, material) \
    { number##u, anchor, owner, kind, material, 1, 1 }

static const DM1_V1_G0651G0700SourceAuditPc34 k_audit[] = {
    ROW(651, "DEFS.H:6259", "csb_v1_runtime_pc34_compat", DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(652, "DEFS.H:6263", "fail_closed: GEM platform allocator boundary", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(653, "MEMORY.C:66", "fail_closed: GEM platform allocator boundary", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(654, "MEMORY.C:89", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34, 1),
    ROW(655, "DEFS.H:6266", "fail_closed: no verified ReDMCSB block-list storage owner", DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34, 0),
    ROW(656, "MEMORY.C:92", "csb_v1_runtime_pc34_compat", DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(657, "DEFS.H:6268", "fail_closed: no verified ReDMCSB block-list storage owner", DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34, 0),
    ROW(658, "DEFS.H:6269", "fail_closed: no verified ReDMCSB block-list storage owner", DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34, 0),
    ROW(659, "DEFS.H:6270", "fail_closed: no verified ReDMCSB block-list storage owner", DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34, 0),
    ROW(660, "DEFS.H:6272", "fail_closed: no verified ReDMCSB block-list storage owner", DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34, 0),
    ROW(661, "BASE.C:33", "csb_v1_runtime_pc34_compat", DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(662, "MEMORY.C:100", "csb_v1_runtime_pc34_compat", DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(663, "HINTLZW.C:8", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(664, "HINTLZW.C:9", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(665, "HINTLZW.C:10", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(666, "HINTLZW.C:11", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(667, "HINTLZW.C:12", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(668, "HINTLZW.C:13", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(669, "HINTLZW.C:14", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(670, "HINTLZW.C:26", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(671, "HINTLZW.C:27", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(672, "HINTLZW.C:28", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(673, "HINTLZW.C:29", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(674, "HINTLZW.C:93", "fail_closed: ReDMCSB LZW toolchain state", DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34, 0),
    ROW(675, "DUNVIEW.C:68", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(676, "DUNVIEW.C:69", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(677, "DUNVIEW.C:70", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(678, "DUNVIEW.C:71", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(679, "DUNVIEW.C:72", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(680, "DUNVIEW.C:73", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(681, "DUNVIEW.C:74", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(682, "DUNVIEW.C:75", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(683, "DUNVIEW.C:76", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(684, "DUNVIEW.C:77", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(685, "DUNVIEW.C:78", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(686, "DUNVIEW.C:79", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(687, "DUNVIEW.C:80", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(688, "DUNVIEW.C:81", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(689, "DUNVIEW.C:82", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(690, "DUNVIEW.C:83", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(691, "DUNVIEW.C:84", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(692, "DUNVIEW.C:85", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(693, "DUNVIEW.C:90", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(694, "DUNVIEW.C:91", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(695, "DUNVIEW.C:92", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(696, "DEFS.H:5417", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(697, "DEFS.H:5418", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(698, "DEFS.H:5419", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(699, "DEFS.H:5420", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(700, "DEFS.H:5421", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34, 1)
};

#undef ROW

const DM1_V1_G0651G0700SourceAuditPc34 *
dm1_v1_g0651_g0700_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0651G0700SourceAuditPc34 *
dm1_v1_g0651_g0700_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0651_g0700_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:5417-5421/6259-6272, BASE.C:33, MEMORY.C:66-100, "
           "HINTLZW.C:8-29/93, and DUNVIEW.C:68-92 define G0651-G0700. "
           "Firestaff routes cache and dungeon assets to named owners; original "
           "stairs, doors, and walls require authentic source material. GEM, LZW, "
           "and unverified allocator-list state fail closed. No independent global "
           "ABI, synthetic bitmap, or synthetic media state is created.";
}
