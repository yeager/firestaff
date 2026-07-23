#ifndef FIRESTAFF_DM1_V1_F0886_F0905_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0886_F0905_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0886_F0905_LOCAL_ONLY_PC34 = 0,
    DM1_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34 = 1
} DM1_V1_F0886F0905OwnershipKindPc34;

typedef struct {
    unsigned int number;
    DM1_V1_F0886F0905OwnershipKindPc34 kind;
    const char *actual_symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} DM1_V1_F0886F0905OwnershipPc34;

const DM1_V1_F0886F0905OwnershipPc34 *
dm1_v1_f0886_f0905_source_ownership_pc34(unsigned int number);
int dm1_v1_f0886_f0905_has_standalone_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f0886_f0905_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
