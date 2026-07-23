#ifndef FIRESTAFF_DM1_V1_G0601_G0650_MOUSE_GRAPHICS_MEMORY_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0601_G0650_MOUSE_GRAPHICS_MEMORY_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef enum DM1_V1_G0601G0650OwnerKindPc34 {
    DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34 = 0,
    DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34,
    DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34,
    DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34,
    DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34
} DM1_V1_G0601G0650OwnerKindPc34;

typedef struct DM1_V1_G0601G0650SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_platform_boundary;
    DM1_V1_G0601G0650OwnerKindPc34 owner_kind;
    int authentic_source_material_required;
    int fail_closed_when_unavailable;
    int independent_global_abi_forbidden;
} DM1_V1_G0601G0650SourceAuditPc34;

const DM1_V1_G0601G0650SourceAuditPc34 *
dm1_v1_g0601_g0650_source_audit_pc34(size_t *out_count);
const DM1_V1_G0601G0650SourceAuditPc34 *
dm1_v1_g0601_g0650_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_g0601_g0650_source_audit_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_G0601_G0650_MOUSE_GRAPHICS_MEMORY_SOURCE_AUDIT_PC34_COMPAT_H */
