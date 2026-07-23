#ifndef FIRESTAFF_DM1_V1_G0701_G0750_STARTUP_MEDIA_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0701_G0750_STARTUP_MEDIA_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef enum DM1_V1_G0701G0750OwnerKindPc34 {
    DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34 = 0,
    DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34,
    DM1_V1_G0701G0750_OWNER_INPUT_RUNTIME_PC34,
    DM1_V1_G0701G0750_OWNER_AUDIO_SOURCE_PC34,
    DM1_V1_G0701G0750_OWNER_STARTUP_SOURCE_PC34,
    DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34
} DM1_V1_G0701G0750OwnerKindPc34;

typedef struct DM1_V1_G0701G0750SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    DM1_V1_G0701G0750OwnerKindPc34 owner_kind;
    int authentic_source_material_required;
    int fail_closed_when_unavailable;
    int independent_global_abi_forbidden;
} DM1_V1_G0701G0750SourceAuditPc34;

const DM1_V1_G0701G0750SourceAuditPc34 *
dm1_v1_g0701_g0750_source_audit_pc34(size_t *out_count);
const DM1_V1_G0701G0750SourceAuditPc34 *
dm1_v1_g0701_g0750_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_g0701_g0750_source_audit_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_G0701_G0750_STARTUP_MEDIA_SOURCE_AUDIT_PC34_COMPAT_H */
