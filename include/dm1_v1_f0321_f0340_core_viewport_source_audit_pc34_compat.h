#ifndef FIRESTAFF_DM1_V1_F0321_F0340_CORE_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0321_F0340_CORE_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0321F0340SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    int raw_original_data_required;
    int fail_closed_when_unavailable;
    int audit_only_no_synthetic_render_or_ui;
} DM1_V1_F0321F0340SourceAuditPc34;

const DM1_V1_F0321F0340SourceAuditPc34 *
dm1_v1_f0321_f0340_source_audit_pc34(size_t *out_count);
const DM1_V1_F0321F0340SourceAuditPc34 *
dm1_v1_f0321_f0340_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_f0321_f0340_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0321_F0340_CORE_VIEWPORT_SOURCE_AUDIT_PC34_COMPAT_H */
