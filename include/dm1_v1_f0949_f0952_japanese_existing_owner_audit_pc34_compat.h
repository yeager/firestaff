#ifndef FIRESTAFF_DM1_V1_F0949_F0952_JAPANESE_EXISTING_OWNER_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0949_F0952_JAPANESE_EXISTING_OWNER_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef struct DM1_V1_F0949F0952ExistingOwnerAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    int direct_source_owner;
    int requires_real_pc98_material_or_io;
    int audit_only_no_synthetic_wrapper;
} DM1_V1_F0949F0952ExistingOwnerAuditPc34;

const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *
dm1_v1_f0949_f0952_existing_owner_audit_pc34(size_t *out_count);
const DM1_V1_F0949F0952ExistingOwnerAuditPc34 *
dm1_v1_f0949_f0952_existing_owner_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_f0949_f0952_existing_owner_audit_evidence_pc34(void);

#endif
