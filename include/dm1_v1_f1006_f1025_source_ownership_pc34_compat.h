#ifndef FIRESTAFF_DM1_V1_F1006_F1025_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F1006_F1025_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F1006_F1025_LOCAL_SYMBOL_PC34 = 0,
    DM1_V1_F1006_F1025_SOURCE_OWNER_PC34 = 1,
    DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34 = 2
} DM1_V1_F1006F1025OwnershipKindPc34;

typedef struct {
    unsigned int number;
    DM1_V1_F1006F1025OwnershipKindPc34 kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} DM1_V1_F1006F1025OwnershipPc34;

const DM1_V1_F1006F1025OwnershipPc34 *
dm1_v1_f1006_f1025_source_ownership_pc34(unsigned int number);

/* True only for an existing owner that consumes caller-supplied original data. */
int dm1_v1_f1006_f1025_admits_authentic_route_pc34(unsigned int number);
int dm1_v1_f1006_f1025_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f1006_f1025_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
