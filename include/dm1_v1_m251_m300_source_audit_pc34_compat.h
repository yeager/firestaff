#ifndef FIRESTAFF_DM1_V1_M251_M300_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_M251_M300_SOURCE_AUDIT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *source_anchor;
} Dm1V1M251M300SourceAuditPc34;

const Dm1V1M251M300SourceAuditPc34 *
dm1_v1_m251_m300_source_audit_pc34(unsigned int number);
int dm1_v1_m251_m300_has_verified_owner_pc34(unsigned int number);
int dm1_v1_m251_m300_has_synthetic_route_pc34(void);
const char *dm1_v1_m251_m300_source_audit_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
