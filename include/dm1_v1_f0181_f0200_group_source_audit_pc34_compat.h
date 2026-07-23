#ifndef FIRESTAFF_DM1_V1_F0181_F0200_GROUP_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0181_F0200_GROUP_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0181F0200GroupSourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    int raw_pc34_required;
    int fail_closed_when_unavailable;
} DM1_V1_F0181F0200GroupSourceAuditPc34;

const DM1_V1_F0181F0200GroupSourceAuditPc34 *
dm1_v1_f0181_f0200_group_source_audit_pc34(size_t *out_count);
const DM1_V1_F0181F0200GroupSourceAuditPc34 *
dm1_v1_f0181_f0200_group_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_f0181_f0200_group_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0181_F0200_GROUP_SOURCE_AUDIT_PC34_COMPAT_H */
