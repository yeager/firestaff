#ifndef FIRESTAFF_DM1_V1_C001_C004_ERS_SPECIAL_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_C001_C004_ERS_SPECIAL_SOURCE_AUDIT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_REDMCSB_SYMBOL_VERIFIED_EXISTING_OWNER_PC34 = 0,
    DM1_V1_REDMCSB_SYMBOL_UNVERIFIED_NO_ROUTE_PC34 = 1,
    DM1_V1_REDMCSB_SYMBOL_PLATFORM_OR_ABSENT_PC34 = 2
} Dm1V1RedmcsbSymbolDispositionPc34;

typedef struct {
    const char *symbol;
    const char *source_anchor;
    Dm1V1RedmcsbSymbolDispositionPc34 disposition;
    const char *owner_or_rationale;
} Dm1V1RedmcsbSymbolAuditPc34;

const Dm1V1RedmcsbSymbolAuditPc34 *
dm1_v1_c001_c004_ers_special_source_audit_pc34(unsigned int index);
unsigned int dm1_v1_c001_c004_ers_special_source_audit_count_pc34(void);
int dm1_v1_c001_c004_ers_special_has_synthetic_route_pc34(void);
const char *dm1_v1_c001_c004_ers_special_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
