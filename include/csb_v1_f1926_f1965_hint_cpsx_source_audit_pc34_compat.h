#ifndef FIRESTAFF_CSB_V1_F1926_F1965_HINT_CPSX_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1926_F1965_HINT_CPSX_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CSB_V1_F1926F1965SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner_or_fail_closed_boundary;
    int direct_redmcsb_source_owner;
    int raw_source_or_pc34_material_required;
    int fail_closed_when_unavailable;
    int audit_only_no_synthetic_render_ui_or_timing;
} CSB_V1_F1926F1965SourceAuditPc34;

const CSB_V1_F1926F1965SourceAuditPc34 *
csb_v1_f1926_f1965_source_audit_pc34(size_t *out_count);
const CSB_V1_F1926F1965SourceAuditPc34 *
csb_v1_f1926_f1965_source_audit_find_pc34(unsigned int symbol_number);
const char *csb_v1_f1926_f1965_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1926_F1965_HINT_CPSX_SOURCE_AUDIT_PC34_COMPAT_H */
