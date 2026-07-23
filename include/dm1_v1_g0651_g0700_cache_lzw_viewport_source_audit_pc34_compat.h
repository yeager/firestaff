#ifndef FIRESTAFF_DM1_V1_G0651_G0700_CACHE_LZW_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0651_G0700_CACHE_LZW_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef enum DM1_V1_G0651G0700OwnerKindPc34 {
    DM1_V1_G0651G0700_OWNER_MEMORY_RUNTIME_PC34 = 0,
    DM1_V1_G0651G0700_OWNER_PLATFORM_OR_TOOLCHAIN_BOUNDARY_PC34,
    DM1_V1_G0651G0700_OWNER_UNMAPPED_ALLOCATOR_BOUNDARY_PC34,
    DM1_V1_G0651G0700_OWNER_DUNGEON_ASSET_PC34
} DM1_V1_G0651G0700OwnerKindPc34;

typedef struct DM1_V1_G0651G0700SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    DM1_V1_G0651G0700OwnerKindPc34 owner_kind;
    int authentic_source_material_required;
    int fail_closed_when_unavailable;
    int independent_global_abi_forbidden;
} DM1_V1_G0651G0700SourceAuditPc34;

const DM1_V1_G0651G0700SourceAuditPc34 *
dm1_v1_g0651_g0700_source_audit_pc34(size_t *out_count);
const DM1_V1_G0651G0700SourceAuditPc34 *
dm1_v1_g0651_g0700_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_g0651_g0700_source_audit_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_G0651_G0700_CACHE_LZW_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H */
