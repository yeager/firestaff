#ifndef FIRESTAFF_DM1_V1_F0946_F0965_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0946_F0965_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0946_F0965_PLATFORM_BOUNDARY_PC34 = 0,
    DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34 = 1,
    DM1_V1_F0946_F0965_LOCAL_ONLY_PC34 = 2
} DM1_V1_F0946F0965OwnershipKindPc34;

typedef struct {
    unsigned int number;
    DM1_V1_F0946F0965OwnershipKindPc34 kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} DM1_V1_F0946F0965OwnershipPc34;

const DM1_V1_F0946F0965OwnershipPc34 *
dm1_v1_f0946_f0965_source_ownership_pc34(unsigned int number);
int dm1_v1_f0946_f0965_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f0946_f0965_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
