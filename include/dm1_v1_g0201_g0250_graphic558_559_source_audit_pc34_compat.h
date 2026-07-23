#ifndef FIRESTAFF_DM1_V1_G0201_G0250_GRAPHIC558_559_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0201_G0250_GRAPHIC558_559_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_G0201G0250SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    int raw_source_or_pc34_material_required;
    int fail_closed_when_unavailable;
    int audit_only_no_synthetic_behavior;
} DM1_V1_G0201G0250SourceAuditPc34;

const DM1_V1_G0201G0250SourceAuditPc34 *
dm1_v1_g0201_g0250_source_audit_pc34(size_t *out_count);
const DM1_V1_G0201G0250SourceAuditPc34 *
dm1_v1_g0201_g0250_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_g0201_g0250_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_G0201_G0250_GRAPHIC558_559_SOURCE_AUDIT_PC34_COMPAT_H */
