#ifndef FIRESTAFF_CSB_V1_F1206_F1225_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1206_F1225_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSB_V1_F1206F1225OwnershipKindPc34 {
    CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34 = 0,
    CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34 = 1
} CSB_V1_F1206F1225OwnershipKindPc34;

typedef struct CSB_V1_F1206F1225OwnershipPc34 {
    unsigned int number;
    CSB_V1_F1206F1225OwnershipKindPc34 kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} CSB_V1_F1206F1225OwnershipPc34;

const CSB_V1_F1206F1225OwnershipPc34 *
csb_v1_f1206_f1225_source_ownership_pc34(unsigned int number);
int csb_v1_f1206_f1225_admits_authentic_route_pc34(unsigned int number);
int csb_v1_f1206_f1225_has_synthetic_route_pc34(unsigned int number);
const char *csb_v1_f1206_f1225_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1206_F1225_SOURCE_OWNERSHIP_PC34_COMPAT_H */
