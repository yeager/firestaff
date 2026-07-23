#ifndef FIRESTAFF_CSB_V1_F0886_F0905_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0886_F0905_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_F0886_F0905_LOCAL_ONLY_PC34 = 0,
    CSB_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34 = 1
} CSB_V1_F0886F0905OwnershipKindPc34;

typedef struct {
    unsigned int number;
    CSB_V1_F0886F0905OwnershipKindPc34 kind;
    const char *actual_symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} CSB_V1_F0886F0905OwnershipPc34;

/* Returns source ownership only; this module never draws, delays, or mutates. */
const CSB_V1_F0886F0905OwnershipPc34 *
csb_v1_f0886_f0905_source_ownership_pc34(unsigned int number);

/* No number in this range may create a standalone substitute runtime route. */
int csb_v1_f0886_f0905_has_standalone_synthetic_route_pc34(unsigned int number);

const char *csb_v1_f0886_f0905_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0886_F0905_SOURCE_OWNERSHIP_PC34_COMPAT_H */
