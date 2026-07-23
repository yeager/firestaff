#ifndef FIRESTAFF_DM1_V1_G1000_G1050_VIEWPORT_INPUT_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G1000_G1050_VIEWPORT_INPUT_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef enum DM1_V1_G1000G1050OwnerKindPc34 {
    DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34 = 0,
    DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34,
    DM1_V1_G1000G1050_OWNER_AUDIO_PC34,
    DM1_V1_G1000G1050_OWNER_SAVE_LOAD_PC34,
    DM1_V1_G1000G1050_OWNER_INPUT_UI_PC34,
    DM1_V1_G1000G1050_OWNER_CHAMPION_PANEL_PC34,
    DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34,
    DM1_V1_G1000G1050_OWNER_UNMAPPED_BOUNDARY_PC34
} DM1_V1_G1000G1050OwnerKindPc34;

typedef struct DM1_V1_G1000G1050SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    DM1_V1_G1000G1050OwnerKindPc34 owner_kind;
    int authentic_source_material_required;
    int fail_closed_when_unavailable;
    int independent_global_abi_forbidden;
} DM1_V1_G1000G1050SourceAuditPc34;

const DM1_V1_G1000G1050SourceAuditPc34 *
dm1_v1_g1000_g1050_source_audit_pc34(size_t *out_count);
const DM1_V1_G1000G1050SourceAuditPc34 *
dm1_v1_g1000_g1050_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_g1000_g1050_source_audit_evidence_pc34(void);

#endif /* FIRESTAFF_DM1_V1_G1000_G1050_VIEWPORT_INPUT_SOURCE_AUDIT_PC34_COMPAT_H */
