#ifndef FIRESTAFF_DM1_V1_F2026_F2065_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F2026_F2065_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F2026_F2065_UNASSIGNED_OR_LOCAL_PC34 = 0,
    DM1_V1_F2026_F2065_PLATFORM_HINT_BOUNDARY_PC34 = 1,
    DM1_V1_F2026_F2065_EXISTING_CALLER_INPUT_OWNER_PC34 = 2
} DM1_V1_F2026F2065OwnershipKindPc34;

typedef struct {
    unsigned int number;
    DM1_V1_F2026F2065OwnershipKindPc34 kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} DM1_V1_F2026F2065OwnershipPc34;

const DM1_V1_F2026F2065OwnershipPc34 *
dm1_v1_f2026_f2065_source_ownership_pc34(unsigned int number);
int dm1_v1_f2026_f2065_admits_authentic_route_pc34(unsigned int number);
int dm1_v1_f2026_f2065_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f2026_f2065_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
