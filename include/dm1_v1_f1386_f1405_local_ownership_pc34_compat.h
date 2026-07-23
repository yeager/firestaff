#ifndef FIRESTAFF_DM1_V1_F1386_F1405_LOCAL_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F1386_F1405_LOCAL_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *parent_owner;
    const char *source_anchor;
} DM1_V1_F1386F1405LocalOwnershipPc34;

const DM1_V1_F1386F1405LocalOwnershipPc34 *
dm1_v1_f1386_f1405_local_ownership_pc34(unsigned int number);
int dm1_v1_f1386_f1405_admits_standalone_route_pc34(unsigned int number);
int dm1_v1_f1386_f1405_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f1386_f1405_local_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
