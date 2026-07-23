#ifndef FIRESTAFF_DM1_V1_M201_M250_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_M201_M250_SOURCE_AUDIT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_M201_M250_UNVERIFIED_NO_ROUTE_PC34 = 0,
    DM1_V1_M201_M250_ABSENT_PC34 = 1
} Dm1V1M201M250DispositionPc34;

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *source_anchor;
    Dm1V1M201M250DispositionPc34 disposition;
} Dm1V1M201M250SourceAuditPc34;

const Dm1V1M201M250SourceAuditPc34 *
dm1_v1_m201_m250_source_audit_pc34(unsigned int number);
int dm1_v1_m201_m250_has_verified_owner_pc34(unsigned int number);
int dm1_v1_m201_m250_has_synthetic_route_pc34(void);
const char *dm1_v1_m201_m250_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
