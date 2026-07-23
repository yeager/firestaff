#ifndef FIRESTAFF_DM1_V1_F0931_F0934_PRIM_CHECKSUM_HEX_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0931_F0934_PRIM_CHECKSUM_HEX_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>

typedef struct DM1_V1_F0931F0934SourceAuditPc34 {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    int direct_pc34_owner;
    int fail_closed_when_source_contract_is_unavailable;
    int audit_only_no_synthetic_wrapper;
} DM1_V1_F0931F0934SourceAuditPc34;

const DM1_V1_F0931F0934SourceAuditPc34 *
dm1_v1_f0931_f0934_source_audit_pc34(size_t *out_count);
const DM1_V1_F0931F0934SourceAuditPc34 *
dm1_v1_f0931_f0934_source_audit_find_pc34(unsigned int symbol_number);
const char *dm1_v1_f0931_f0934_source_audit_evidence_pc34(void);

#endif
