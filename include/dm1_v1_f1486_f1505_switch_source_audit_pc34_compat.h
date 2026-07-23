#ifndef FIRESTAFF_DM1_V1_F1486_F1505_SWITCH_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F1486_F1505_SWITCH_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F1486F1505SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    int direct_redmcsb_source_owner;
    int raw_source_or_pc34_material_required;
    int fail_closed_when_unavailable;
    int audit_only_no_synthetic_render_ui_or_timing;
} DM1_V1_F1486F1505SourceAuditPc34;

const DM1_V1_F1486F1505SourceAuditPc34 *
dm1_v1_f1486_f1505_source_audit_pc34(size_t *out_count);
const DM1_V1_F1486F1505SourceAuditPc34 *
dm1_v1_f1486_f1505_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_f1486_f1505_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F1486_F1505_SWITCH_SOURCE_AUDIT_PC34_COMPAT_H */
