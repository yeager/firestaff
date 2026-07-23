#ifndef FIRESTAFF_REDMCSB_L0051_L0100_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_L0051_L0100_SOURCE_AUDIT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *source_anchor;
    const char *owner;
} RedmcsbL0051L0100SourceAuditPc34;

const RedmcsbL0051L0100SourceAuditPc34 *
redmcsb_l0051_l0100_source_audit_pc34(unsigned int index);
unsigned int redmcsb_l0051_l0100_source_audit_count_pc34(void);
int redmcsb_l0051_l0100_has_verified_owner_pc34(unsigned int index);
int redmcsb_l0051_l0100_has_synthetic_route_pc34(void);
const char *redmcsb_l0051_l0100_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
